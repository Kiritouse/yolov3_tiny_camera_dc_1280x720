/****************************************************************************
*   Generated by ACUITY 6.12.4
*   Match ovxlib 1.1.53
*
*   Neural Network appliction post-process header file
****************************************************************************/
#ifndef _VNN_POST_PROCESS_H_
#define _VNN_POST_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

vsi_status vnn_PostProcessYolov3TinyUint8(struct Vnn_Wrapper *vnn_wrapper, detected_list* list);

const vsi_nn_postprocess_map_element_t * vnn_GetPostProcessMap(data_file_type data_type);

uint32_t vnn_GetPostProcessMapCount(data_file_type data_type);

#ifdef __cplusplus
}
#endif

#endif
