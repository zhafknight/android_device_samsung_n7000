/*
 * Vendor-side compatibility layer for the legacy BCM4751 gpsd binary.
 *
 * gpsd was linked against the pre-Treble C++ libsensor ABI.  LOS 23.2 does
 * not permit a vendor process to load that platform library.  Export the
 * small ABI surface gpsd imports and forward it to libsensorndkbridge, which
 * is the vendor-safe NDK sensor client.
 */

#define LOG_TAG "libdmitry"

#include <android/looper.h>
#include <android/sensor.h>
#include <utils/RefBase.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/* Exported by libsensorndkbridge, but omitted from the public NDK header. */
extern "C" float ASensor_getMaxRange(const ASensor* sensor);

namespace {

/*
 * gpsd only sees Sensor through the methods exported below.  The first two
 * fields deliberately have String8's one-pointer ABI so getName()/getVendor()
 * can return them as const android::String8& without linking platform libutils.
 */
struct LegacySensor {
    const char* name;
    const char* vendor;
    const ASensor* sensor;
};

struct ManagerState {
    void* legacy_manager;
    ASensorManager* manager;
    LegacySensor* sensors;
    const LegacySensor** sensor_list;
    size_t sensor_count;
    ManagerState* next;
};

ManagerState* gManagers;

ManagerState* findManager(void* legacy_manager) {
    for (ManagerState* state = gManagers; state; state = state->next) {
        if (state->legacy_manager == legacy_manager) return state;
    }
    return nullptr;
}

LegacySensor* findSensor(ManagerState* state, const ASensor* sensor) {
    if (!state || !sensor) return nullptr;
    for (size_t i = 0; i < state->sensor_count; ++i) {
        if (state->sensors[i].sensor == sensor) return &state->sensors[i];
    }
    return nullptr;
}

bool populateSensors(ManagerState* state) {
    if (!state) return false;
    if (state->sensor_list) return true;

    ASensorList sensors = nullptr;
    const int count = ASensorManager_getSensorList(state->manager, &sensors);
    if (count < 0) return false;

    state->sensor_count = static_cast<size_t>(count);
    state->sensors = static_cast<LegacySensor*>(calloc(state->sensor_count, sizeof(*state->sensors)));
    state->sensor_list = static_cast<const LegacySensor**>(calloc(state->sensor_count, sizeof(*state->sensor_list)));
    if ((state->sensor_count && !state->sensors) || (state->sensor_count && !state->sensor_list)) return false;

    for (size_t i = 0; i < state->sensor_count; ++i) {
        state->sensors[i].sensor = sensors[i];
        state->sensors[i].name = ASensor_getName(sensors[i]);
        state->sensors[i].vendor = ASensor_getVendor(sensors[i]);
        state->sensor_list[i] = &state->sensors[i];
    }
    return true;
}

class LegacyEventQueue : public android::RefBase {
  public:
    explicit LegacyEventQueue(ASensorManager* manager) : manager_(manager) {
        pipefd_[0] = -1;
        pipefd_[1] = -1;
        pthread_mutex_init(&lock_, nullptr);
        pthread_cond_init(&ready_, nullptr);
    }

    bool start() {
        if (pipe(pipefd_) != 0) return false;
        fcntl(pipefd_[0], F_SETFL, fcntl(pipefd_[0], F_GETFL) | O_NONBLOCK);
        fcntl(pipefd_[1], F_SETFL, fcntl(pipefd_[1], F_GETFL) | O_NONBLOCK);
        if (pthread_create(&thread_, nullptr, &LegacyEventQueue::threadMain, this) != 0) return false;

        pthread_mutex_lock(&lock_);
        while (!is_ready_) pthread_cond_wait(&ready_, &lock_);
        const bool started = queue_ != nullptr;
        pthread_mutex_unlock(&lock_);
        return started;
    }

    ~LegacyEventQueue() override {
        pthread_mutex_lock(&lock_);
        stop_ = true;
        if (looper_) ALooper_wake(looper_);
        pthread_mutex_unlock(&lock_);
        if (thread_) pthread_join(thread_, nullptr);
        if (pipefd_[0] >= 0) close(pipefd_[0]);
        if (pipefd_[1] >= 0) close(pipefd_[1]);
        pthread_cond_destroy(&ready_);
        pthread_mutex_destroy(&lock_);
    }

    ASensorEventQueue* queue() const { return queue_; }
    int fd() const { return pipefd_[0]; }

    void consumeNotifications() {
        uint8_t buffer[32];
        while (read(pipefd_[0], buffer, sizeof(buffer)) > 0) {}
    }

  private:
    static int queueCallback(int, int, void* data) {
        LegacyEventQueue* self = static_cast<LegacyEventQueue*>(data);
        const uint8_t marker = 1;
        (void)write(self->pipefd_[1], &marker, sizeof(marker));
        return 1;  // ALOOPER_CALLBACK_KEEP
    }

    static void* threadMain(void* data) {
        LegacyEventQueue* self = static_cast<LegacyEventQueue*>(data);
        ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        ASensorEventQueue* queue = looper
                ? ASensorManager_createEventQueue(self->manager_, looper, 1,
                        &LegacyEventQueue::queueCallback, self)
                : nullptr;

        pthread_mutex_lock(&self->lock_);
        self->looper_ = looper;
        self->queue_ = queue;
        self->is_ready_ = true;
        pthread_cond_signal(&self->ready_);
        pthread_mutex_unlock(&self->lock_);

        while (queue) {
            pthread_mutex_lock(&self->lock_);
            const bool stop = self->stop_;
            pthread_mutex_unlock(&self->lock_);
            if (stop) break;
            ALooper_pollOnce(-1, nullptr, nullptr, nullptr);
        }

        if (queue) ASensorManager_destroyEventQueue(self->manager_, queue);
        /* The looper is thread-local and is released when this worker exits. */
        return nullptr;
    }

    ASensorManager* manager_;
    ALooper* looper_ = nullptr;
    ASensorEventQueue* queue_ = nullptr;
    int pipefd_[2];
    pthread_t thread_{};
    pthread_mutex_t lock_;
    pthread_cond_t ready_;
    bool is_ready_ = false;
    bool stop_ = false;
};

}  // namespace

/* gpsd's old android::Singleton<android::SensorManager>::sInstance. */
extern "C" void* legacy_sensor_manager_singleton
        __asm__("_ZN7android9SingletonINS_13SensorManagerEE9sInstanceE");
void* legacy_sensor_manager_singleton = nullptr;

extern "C" void legacy_sensor_manager_ctor(void* legacy_manager)
        __asm__("_ZN7android13SensorManagerC1Ev");
void legacy_sensor_manager_ctor(void* legacy_manager) {
    if (findManager(legacy_manager)) return;
    ManagerState* state = static_cast<ManagerState*>(calloc(1, sizeof(*state)));
    if (!state) return;
    state->legacy_manager = legacy_manager;
    state->manager = ASensorManager_getInstanceForPackage("gpsd");
    state->next = gManagers;
    gManagers = state;
}

extern "C" ssize_t legacy_sensor_manager_get_list(void* legacy_manager, const LegacySensor*** list)
        __asm__("_ZN7android13SensorManager13getSensorListEPPKPKNS_6SensorE");
ssize_t legacy_sensor_manager_get_list(void* legacy_manager, const LegacySensor*** list) {
    ManagerState* state = findManager(legacy_manager);
    if (!state || !populateSensors(state)) return -ENODEV;
    *list = state->sensor_list;
    return static_cast<ssize_t>(state->sensor_count);
}

extern "C" const LegacySensor* legacy_sensor_manager_get_default(void* legacy_manager, int type)
        __asm__("_ZN7android13SensorManager16getDefaultSensorEi");
const LegacySensor* legacy_sensor_manager_get_default(void* legacy_manager, int type) {
    ManagerState* state = findManager(legacy_manager);
    if (!state || !populateSensors(state)) return nullptr;
    return findSensor(state, ASensorManager_getDefaultSensor(state->manager, type));
}

extern "C" void legacy_sensor_manager_create_queue(void** out, void* legacy_manager)
        __asm__("_ZN7android13SensorManager16createEventQueueEv");
void legacy_sensor_manager_create_queue(void** out, void* legacy_manager) {
    *out = nullptr;
    ManagerState* state = findManager(legacy_manager);
    if (!state) return;

    LegacyEventQueue* legacy_queue = new LegacyEventQueue(state->manager);
    if (!legacy_queue->start()) {
        delete legacy_queue;
        return;
    }
    legacy_queue->incStrong(out);
    *out = legacy_queue;
}

extern "C" int legacy_queue_get_fd(const LegacyEventQueue* queue)
        __asm__("_ZNK7android16SensorEventQueue5getFdEv");
int legacy_queue_get_fd(const LegacyEventQueue* queue) { return queue ? queue->fd() : -1; }

extern "C" ssize_t legacy_queue_read(LegacyEventQueue* queue, ASensorEvent* events, unsigned count)
        __asm__("_ZN7android16SensorEventQueue4readEP12ASensorEventj");
ssize_t legacy_queue_read(LegacyEventQueue* queue, ASensorEvent* events, unsigned count) {
    if (!queue || !queue->queue()) return -ENODEV;
    queue->consumeNotifications();
    return ASensorEventQueue_getEvents(queue->queue(), events, count);
}

extern "C" int legacy_queue_enable(const LegacyEventQueue* queue, const LegacySensor* sensor)
        __asm__("_ZNK7android16SensorEventQueue12enableSensorEPKNS_6SensorE");
int legacy_queue_enable(const LegacyEventQueue* queue, const LegacySensor* sensor) {
    return queue && sensor ? ASensorEventQueue_enableSensor(queue->queue(), sensor->sensor) : -EINVAL;
}

extern "C" int legacy_queue_disable(const LegacyEventQueue* queue, const LegacySensor* sensor)
        __asm__("_ZNK7android16SensorEventQueue13disableSensorEPKNS_6SensorE");
int legacy_queue_disable(const LegacyEventQueue* queue, const LegacySensor* sensor) {
    return queue && sensor ? ASensorEventQueue_disableSensor(queue->queue(), sensor->sensor) : -EINVAL;
}

extern "C" int legacy_queue_set_rate(const LegacyEventQueue* queue, const LegacySensor* sensor, int64_t ns)
        __asm__("_ZNK7android16SensorEventQueue12setEventRateEPKNS_6SensorEx");
int legacy_queue_set_rate(const LegacyEventQueue* queue, const LegacySensor* sensor, int64_t ns) {
    if (!queue || !sensor) return -EINVAL;
    const int64_t us = ns / 1000;
    return ASensorEventQueue_setEventRate(queue->queue(), sensor->sensor,
            us > INT32_MAX ? INT32_MAX : static_cast<int32_t>(us));
}

extern "C" const char* legacy_sensor_get_name(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor7getNameEv");
const char* legacy_sensor_get_name(const LegacySensor* sensor) { return sensor ? sensor->name : nullptr; }

extern "C" const char* legacy_sensor_get_vendor(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor9getVendorEv");
const char* legacy_sensor_get_vendor(const LegacySensor* sensor) { return sensor ? sensor->vendor : nullptr; }

extern "C" int legacy_sensor_get_handle(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor9getHandleEv");
int legacy_sensor_get_handle(const LegacySensor* sensor) { return sensor ? ASensor_getHandle(sensor->sensor) : -1; }

extern "C" int legacy_sensor_get_type(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor7getTypeEv");
int legacy_sensor_get_type(const LegacySensor* sensor) { return sensor ? ASensor_getType(sensor->sensor) : ASENSOR_TYPE_INVALID; }

extern "C" float legacy_sensor_get_max(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor11getMaxValueEv");
float legacy_sensor_get_max(const LegacySensor* sensor) { return sensor ? ASensor_getMaxRange(sensor->sensor) : 0.0f; }

extern "C" float legacy_sensor_get_resolution(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor13getResolutionEv");
float legacy_sensor_get_resolution(const LegacySensor* sensor) { return sensor ? ASensor_getResolution(sensor->sensor) : 0.0f; }

extern "C" float legacy_sensor_get_power(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor13getPowerUsageEv");
float legacy_sensor_get_power(const LegacySensor*) {
    /* The stable vendor NDK does not expose the legacy power metadata. */
    return 0.0f;
}

extern "C" int legacy_sensor_get_min_delay(const LegacySensor* sensor)
        __asm__("_ZNK7android6Sensor11getMinDelayEv");
int legacy_sensor_get_min_delay(const LegacySensor* sensor) { return sensor ? ASensor_getMinDelay(sensor->sensor) : 0; }
