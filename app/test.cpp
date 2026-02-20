#include "test.h"
#include "cmsis_os.h"
#include "dtm.h"

DTM_DEFINE_TOPIC(float, test1);
//DTM_AUTO_REGISTER_TOPIC(float, test1);
extern "C"{

void test_task(void const* argument) {
    dtm::Manager::init();
    DTM_REGISTER_TOPIC(float, test1);
    float value = 0.0f;

    while (1) {
        value += 0.1f;
        if (value > 10.0f) {
            value = 0.0f;
        }
        DTM_PUBLISH(test1, value);

        osDelay(100);
    }
}

}
