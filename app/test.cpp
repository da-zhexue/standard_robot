#include "test.h"
#include "cmsis_os.h"
#include "dtm.h"

DTM_DEFINE_TOPIC(float, test1);
//DTM_AUTO_REGISTER_TOPIC(float, test1);
extern "C"{

void test_task(void const* argument) { // 在FreeRTOS的任务函数所在的cpp文件中不能包含dtm相关函数，
    //否则在freertos.c中只能extern该函数而不能引用头文件
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
