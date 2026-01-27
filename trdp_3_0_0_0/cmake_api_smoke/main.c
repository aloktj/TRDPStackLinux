#include <trdp/trdp_if_light.h>
#include <trdp/tau_ctrl_types.h>

int main(void)
{
    TRDP_ERR_T trdp_result = tlc_init(NULL, NULL, NULL);
    TRDP_ETB_CTRL_T tau_control = {0};

    (void)trdp_result;
    (void)tau_control;

    return 0;
}
