/*
 * Copyright (C) 2015-2020 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Parameters"

#include "iutils/Errors.h"
#include "iutils/CameraLog.h"
#include "iutils/CameraDump.h"
#include "iutils/Utils.h"

#include "PlatformData.h"
#include "Parameters.h"
#include "ParameterHelper.h"

using std::vector;

namespace icamera {

Parameters::Parameters() : mData(ParameterHelper::createParameterData()) {}

Parameters::Parameters(const Parameters& other) :
        mData(ParameterHelper::createParameterData(other.mData)) {}

Parameters& Parameters::operator=(const Parameters& other)
{
    ParameterHelper::AutoWLock wl(mData);
    ParameterHelper::deepCopy(other.mData, mData);
    return *this;
}

Parameters::~Parameters()
{
    ParameterHelper::releaseParameterData(mData);
    mData = nullptr;
}

void Parameters::merge(const Parameters& other)
{
    ParameterHelper::merge(other, this);
}

int Parameters::setAeMode(camera_ae_mode_t aeMode)
{
    uint8_t mode = aeMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AE_MODE, &mode, 1);
}

int Parameters::getAeMode(camera_ae_mode_t& aeMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    aeMode = (camera_ae_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAeState(camera_ae_state_t aeState)
{
    uint8_t state = aeState;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AE_STATE, &state, 1);
}

int Parameters::getAeState(camera_ae_state_t& aeState) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_STATE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    aeState = (camera_ae_state_t)entry.data.u8[0];
    return OK;
}

static int setRegions(CameraMetadata& metadata, camera_window_list_t regions, int tag)
{
    if (regions.empty()) {
        // Nothing to do with an empty parameter.
        return INVALID_OPERATION;
    }

    const int ELEM_NUM = sizeof(camera_window_t) / sizeof(int);
    int values[regions.size() * ELEM_NUM];
    for (size_t i = 0; i < regions.size(); i++) {
        values[i * ELEM_NUM] = regions[i].left;
        values[i * ELEM_NUM + 1] = regions[i].top;
        values[i * ELEM_NUM + 2] = regions[i].right;
        values[i * ELEM_NUM + 3] = regions[i].bottom;
        values[i * ELEM_NUM + 4] = regions[i].weight;
    }

    return metadata.update(tag, values, ARRAY_SIZE(values));
}

static int getRegions(icamera_metadata_ro_entry_t entry, camera_window_list_t& regions)
{
    regions.clear();
    const int ELEM_NUM = sizeof(camera_window_t) / sizeof(int);
    if (entry.count == 0 || entry.count % ELEM_NUM != 0) {
        return NAME_NOT_FOUND;
    }

    camera_window_t w;
    for (size_t i = 0; i < entry.count; i += ELEM_NUM) {
        w.left = entry.data.i32[i];
        w.top = entry.data.i32[i + 1];
        w.right = entry.data.i32[i + 2];
        w.bottom = entry.data.i32[i + 3];
        w.weight = entry.data.i32[i + 4];
        regions.push_back(w);
    }

    return OK;
}

int Parameters::setAeRegions(camera_window_list_t aeRegions)
{
    ParameterHelper::AutoWLock wl(mData);
    return setRegions(ParameterHelper::getMetadata(mData), aeRegions, CAMERA_AE_REGIONS);
}

int Parameters::getAeRegions(camera_window_list_t& aeRegions) const
{
    ParameterHelper::AutoRLock rl(mData);
    return getRegions(ParameterHelper::getMetadataEntry(mData, CAMERA_AE_REGIONS), aeRegions);
}

int Parameters::setAeLock(bool lock)
{
    uint8_t lockValue = lock ? 1 : 0;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AE_LOCK, &lockValue, 1);
}

int Parameters::getAeLock(bool& lock) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_LOCK);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    lock = entry.data.u8[0];
    return OK;
}

int Parameters::setExposureTime(int64_t exposureTime)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_SENSOR_EXPOSURE_TIME, &exposureTime, 1);
}

int Parameters::getExposureTime(int64_t& exposureTime) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SENSOR_EXPOSURE_TIME);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    exposureTime = entry.data.i64[0];
    return OK;
}

int Parameters::setSensitivityGain(float gain)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_SENSITIVITY_GAIN, &gain, 1);
}

int Parameters::getSensitivityGain(float& gain) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_SENSITIVITY_GAIN);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    gain = entry.data.f[0];
    return OK;
}

int Parameters::setSensitivityIso(int32_t iso)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_SENSOR_SENSITIVITY, &iso, 1);
}

int Parameters::getSensitivityIso(int32_t& iso) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SENSOR_SENSITIVITY);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    iso = entry.data.i32[0];
    return OK;
}

int Parameters::setAeCompensation(int ev)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AE_COMPENSATION, &ev, 1);
}

int Parameters::getAeCompensation(int& ev) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_COMPENSATION);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    ev = entry.data.i32[0];
    return OK;
}

int Parameters::setFrameRate(float fps)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_FRAME_RATE, &fps, 1);
}

int Parameters::getFrameRate(float& fps) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_FRAME_RATE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    fps = entry.data.f[0];
    return OK;
}

int Parameters::setAntiBandingMode(camera_antibanding_mode_t bandingMode)
{
    uint8_t mode = bandingMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AE_ANTIBANDING_MODE, &mode, 1);
}

int Parameters::getAntiBandingMode(camera_antibanding_mode_t& bandingMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_ANTIBANDING_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    bandingMode = (camera_antibanding_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAwbMode(camera_awb_mode_t awbMode)
{
    uint8_t mode = awbMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_MODE, &mode, 1);
}

int Parameters::getAwbMode(camera_awb_mode_t& awbMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    awbMode = (camera_awb_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAwbState(camera_awb_state_t awbState)
{
    uint8_t state = awbState;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_STATE, &state, 1);
}

int Parameters::getAwbState(camera_awb_state_t& awbState) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_STATE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    awbState = (camera_awb_state_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAwbLock(bool lock)
{
    uint8_t lockValue = lock ? 1 : 0;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_LOCK, &lockValue, 1);
}

int Parameters::getAwbLock(bool& lock) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_LOCK);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    lock = entry.data.u8[0];
    return OK;
}

int Parameters::setAwbCctRange(camera_range_t cct)
{
    int range[] = {(int)cct.min, (int)cct.max};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_CCT_RANGE, range, ARRAY_SIZE(range));
}

int Parameters::getAwbCctRange(camera_range_t& cct) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_CCT_RANGE);
    const size_t ELEM_NUM = sizeof(camera_range_t) / sizeof(int);
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }
    cct.min = entry.data.i32[0];
    cct.max = entry.data.i32[1];
    return OK;
}

int Parameters::setAwbGains(camera_awb_gains_t awbGains)
{
    int values[] = {awbGains.r_gain, awbGains.g_gain, awbGains.b_gain};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_GAINS, values, ARRAY_SIZE(values));
}

int Parameters::getAwbGains(camera_awb_gains_t& awbGains) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_GAINS);
    const size_t ELEM_NUM = sizeof(camera_awb_gains_t) / sizeof(int);
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }
    awbGains.r_gain = entry.data.i32[0];
    awbGains.g_gain = entry.data.i32[1];
    awbGains.b_gain = entry.data.i32[2];
    return OK;
}

int Parameters::setAwbResult(void* data)
{
    uint32_t size = sizeof(camera_awb_result_t);
    uint32_t tag = CAMERA_AWB_RESULT;
    ParameterHelper::AutoWLock wl(mData);

    if (data == NULL) {
        return ParameterHelper::getMetadata(mData).erase(tag);
    }
    return ParameterHelper::getMetadata(mData).update(tag, (uint8_t*)data, size);
}

int Parameters::getAwbResult(void* data) const
{
    if (data == NULL) {
        return BAD_VALUE;
    }

    uint32_t size = sizeof(camera_awb_result_t);
    uint32_t tag = CAMERA_AWB_RESULT;
    ParameterHelper::AutoRLock rl(mData);

    auto entry = ParameterHelper::getMetadataEntry(mData, tag);
    if (entry.count != size) {
        return NAME_NOT_FOUND;
    }

    MEMCPY_S(data, size, entry.data.u8, size);

    return OK;
}

int Parameters::setAwbGainShift(camera_awb_gains_t awbGainShift)
{
    int values[] = {awbGainShift.r_gain, awbGainShift.g_gain, awbGainShift.b_gain};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_GAIN_SHIFT, values, ARRAY_SIZE(values));
}

int Parameters::getAwbGainShift(camera_awb_gains_t& awbGainShift) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_GAIN_SHIFT);
    const size_t ELEM_NUM = sizeof(camera_awb_gains_t) / sizeof(int);
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }
    awbGainShift.r_gain = entry.data.i32[0];
    awbGainShift.g_gain = entry.data.i32[1];
    awbGainShift.b_gain = entry.data.i32[2];
    return OK;
}

int Parameters::setAwbWhitePoint(camera_coordinate_t whitePoint)
{
    int values[] = {whitePoint.x, whitePoint.y};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_WHITE_POINT, values, ARRAY_SIZE(values));
}

int Parameters::getAwbWhitePoint(camera_coordinate_t& whitePoint) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_WHITE_POINT);
    const size_t ELEM_NUM = sizeof(camera_coordinate_t) / sizeof(int);
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }

    whitePoint.x = entry.data.i32[0];
    whitePoint.y = entry.data.i32[1];

    return OK;
}

int Parameters::setColorTransform(camera_color_transform_t colorTransform)
{
    float* transform = (float*)colorTransform.color_transform;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_COLOR_TRANSFORM, transform, 3 * 3);
}

int Parameters::getColorTransform(camera_color_transform_t& colorTransform) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_COLOR_TRANSFORM);
    const size_t ELEM_NUM = 3 * 3;
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }
    for (size_t i = 0; i < ELEM_NUM; i++) {
        colorTransform.color_transform[i / 3][i % 3] = entry.data.f[i];
    }

    return OK;
}

int Parameters::setColorGains(camera_color_gains_t colorGains)
{
    float* gains = colorGains.color_gains_rggb;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_COLOR_GAINS, gains, 4);
}

int Parameters::getColorGains(camera_color_gains_t& colorGains) const
{
    ParameterHelper::AutoRLock rl(mData);
    icamera_metadata_ro_entry_t entry =
        ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_COLOR_GAINS);
    const size_t ELEM_NUM = 4;
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }
    for (size_t i = 0; i < ELEM_NUM; i++) {
        colorGains.color_gains_rggb[i] = entry.data.f[i];
    }
    return OK;
}

int Parameters::setAwbRegions(camera_window_list_t awbRegions)
{
    ParameterHelper::AutoWLock wl(mData);
    return setRegions(ParameterHelper::getMetadata(mData), awbRegions, CAMERA_AWB_REGIONS);
}

int Parameters::getAwbRegions(camera_window_list_t& awbRegions) const
{
    ParameterHelper::AutoRLock rl(mData);
    return getRegions(ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_REGIONS), awbRegions);
}

int Parameters::setEdgeMode(camera_edge_mode_t edgeMode)
{
    uint8_t mode = edgeMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_EDGE_MODE, &mode, 1);
}

int Parameters::getEdgeMode(camera_edge_mode_t& edgeMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_EDGE_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    edgeMode = (camera_edge_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setNrMode(camera_nr_mode_t nrMode)
{
    uint8_t mode = nrMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_NR_MODE, &mode, 1);
}

int Parameters::getNrMode(camera_nr_mode_t& nrMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_NR_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    nrMode = (camera_nr_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setNrLevel(camera_nr_level_t level)
{
    int values [] = {level.overall, level.spatial, level.temporal};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_NR_LEVEL, values, ARRAY_SIZE(values));
}

int Parameters::getNrLevel(camera_nr_level_t& level) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_NR_LEVEL);
    const size_t ELEM_NUM = sizeof(camera_nr_level_t) / sizeof(int);
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }
    level.overall = entry.data.i32[0];
    level.spatial = entry.data.i32[1];
    level.temporal = entry.data.i32[2];
    return OK;
}

int Parameters::setIrisMode(camera_iris_mode_t irisMode)
{
    uint8_t mode = irisMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_IRIS_MODE, &mode, 1);
}

int Parameters::getIrisMode(camera_iris_mode_t& irisMode)
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_IRIS_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    irisMode = (camera_iris_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setIrisLevel(int level)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_IRIS_LEVEL, &level, 1);
}

int Parameters::getIrisLevel(int& level)
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_IRIS_LEVEL);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    level = entry.data.i32[0];
    return OK;
}

int Parameters::setWdrLevel(uint8_t level)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_WDR_LEVEL, &level, 1);
}

int Parameters::getWdrLevel(uint8_t& level) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_WDR_LEVEL);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    level = entry.data.u8[0];
    return OK;
}

int Parameters::setEffectSceneMode(camera_scene_mode_t sceneMode)
{
    uint8_t mode = sceneMode;
    LOGW("Effect scene mode is deprecated. Please use setSceneMode() instead.");
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_SCENE_MODE, &mode, 1);
}

int Parameters::getEffectSceneMode(camera_scene_mode_t& sceneMode) const
{
    LOGW("Effect scene mode is deprecated. Please use getSceneMode() instead.");
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_SCENE_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    sceneMode = (camera_scene_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setSceneMode(camera_scene_mode_t sceneMode)
{
    uint8_t mode = sceneMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_SCENE_MODE, &mode, 1);
}

int Parameters::getSceneMode(camera_scene_mode_t& sceneMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_SCENE_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    sceneMode = (camera_scene_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setWeightGridMode(camera_weight_grid_mode_t weightGridMode)
{
    uint8_t mode = (uint8_t)weightGridMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_WEIGHT_GRID_MODE, &mode, 1);
}

int Parameters::getWeightGridMode(camera_weight_grid_mode_t& weightGridMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_WEIGHT_GRID_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    weightGridMode = (camera_weight_grid_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setBlcAreaMode(camera_blc_area_mode_t blcAreaMode)
{
    uint8_t mode = blcAreaMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_BLC_AREA_MODE, &mode, 1);
}

int Parameters::getBlcAreaMode(camera_blc_area_mode_t& blcAreaMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_BLC_AREA_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    blcAreaMode = (camera_blc_area_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setFpsRange(camera_range_t fpsRange)
{
    float range[] = {fpsRange.min, fpsRange.max};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AE_TARGET_FPS_RANGE, range, ARRAY_SIZE(range));
}

int Parameters::getFpsRange(camera_range_t& fpsRange) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_TARGET_FPS_RANGE);
    const size_t ELEM_NUM = sizeof(camera_range_t) / sizeof(float);
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }
    fpsRange.min = entry.data.f[0];
    fpsRange.max = entry.data.f[1];
    return OK;
}

int Parameters::setImageEnhancement(camera_image_enhancement_t effects)
{
    int values[] = {effects.sharpness, effects.brightness, effects.contrast, effects.hue, effects.saturation};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_IMAGE_ENHANCEMENT, values, ARRAY_SIZE(values));
}

int Parameters::getImageEnhancement(camera_image_enhancement_t& effects) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_IMAGE_ENHANCEMENT);
    size_t number_of_effects = sizeof(camera_image_enhancement_t) / sizeof(int);
    if (entry.count != number_of_effects) {
        return NAME_NOT_FOUND;
    }
    effects.sharpness = entry.data.i32[0];
    effects.brightness = entry.data.i32[1];
    effects.contrast = entry.data.i32[2];
    effects.hue = entry.data.i32[3];
    effects.saturation = entry.data.i32[4];

    return OK;
}

int Parameters::setDeinterlaceMode(camera_deinterlace_mode_t deinterlaceMode)
{
    uint8_t mode = deinterlaceMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_DEINTERLACE_MODE, &mode, 1);
}

int Parameters::getDeinterlaceMode(camera_deinterlace_mode_t &deinterlaceMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_DEINTERLACE_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    deinterlaceMode = (camera_deinterlace_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::getSupportedVideoStabilizationMode(camera_video_stabilization_list_t &supportedModes) const
{
    supportedModes.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        supportedModes.push_back((camera_video_stabilization_mode_t)entry.data.u8[i]);
    }

    return OK;
}

int Parameters::getSupportedAeMode(vector <camera_ae_mode_t> &supportedAeModes) const
{
    supportedAeModes.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_AVAILABLE_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        supportedAeModes.push_back((camera_ae_mode_t)entry.data.u8[i]);
    }

    return OK;
}

int Parameters::getSupportedAwbMode(vector <camera_awb_mode_t> &supportedAwbModes) const
{
    supportedAwbModes.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_AVAILABLE_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        supportedAwbModes.push_back((camera_awb_mode_t)entry.data.u8[i]);
    }

    return OK;
}

int Parameters::getSupportedAfMode(vector <camera_af_mode_t> &supportedAfModes) const
{
    supportedAfModes.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AF_AVAILABLE_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        supportedAfModes.push_back((camera_af_mode_t)entry.data.u8[i]);
    }

    return OK;
}

int Parameters::getSupportedSceneMode(vector <camera_scene_mode_t> &supportedSceneModes) const
{
    supportedSceneModes.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_CONTROL_AVAILABLE_SCENE_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        supportedSceneModes.push_back((camera_scene_mode_t)entry.data.u8[i]);
    }

    return OK;
}

int Parameters::getSupportedAntibandingMode(vector <camera_antibanding_mode_t> &supportedAntibindingModes) const
{
    supportedAntibindingModes.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_AVAILABLE_ANTIBANDING_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        supportedAntibindingModes.push_back((camera_antibanding_mode_t)entry.data.u8[i]);
    }

    return OK;
}

int Parameters::getSupportedFpsRange(camera_range_array_t& ranges) const
{
    ranges.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_AVAILABLE_TARGET_FPS_RANGES);
    if (entry.count == 0 || entry.count % 2 != 0) {
        return NAME_NOT_FOUND;
    }

    camera_range_t fps;
    for (size_t i = 0; i < entry.count; i += 2) {
        fps.min = entry.data.f[i];
        fps.max = entry.data.f[i + 1];
        ranges.push_back(fps);
    }

    return OK;
}

int Parameters::getSupportedStreamConfig(stream_array_t& config) const
{
    config.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_INFO_AVAILABLE_CONFIGURATIONS);
    const int streamConfMemberNum = sizeof(stream_t) / sizeof(int);
    if (entry.count == 0 || entry.count % streamConfMemberNum != 0) {
        return NAME_NOT_FOUND;
    }

    stream_t cfg;
    CLEAR(cfg);

    for (size_t i = 0; i < entry.count; i += streamConfMemberNum) {
        MEMCPY_S(&cfg, sizeof(stream_t), &entry.data.i32[i], sizeof(stream_t));
        cfg.stride = CameraUtils::getStride(cfg.format, cfg.width);
        cfg.size   = CameraUtils::getFrameSize(cfg.format, cfg.width, cfg.height);
        config.push_back(cfg);
    }
    return OK;
}

int Parameters::getSupportedSensorExposureTimeRange(camera_range_t& range) const
{
    CLEAR(range);
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    if (entry.count != 2) {
        return NAME_NOT_FOUND;
    }

    range.min = (float)(entry.data.i64[0]);
    range.max = (float)(entry.data.i64[1]);
    return OK;
}

int Parameters::getSupportedSensorSensitivityRange(camera_range_t& range) const
{
    CLEAR(range);
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SENSOR_INFO_SENSITIVITY_RANGE);
    if (entry.count != 2) {
        return NAME_NOT_FOUND;
    }

    range.min = entry.data.i32[0];
    range.max = entry.data.i32[1];
    return OK;
}

int Parameters::getSupportedFeatures(camera_features_list_t& features) const
{
    features.clear();
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_INFO_AVAILABLE_FEATURES);
    for (size_t i = 0; i < entry.count; i++) {
        features.push_back((camera_features)entry.data.u8[i]);
    }
    return OK;
}

int Parameters::getAeCompensationRange(camera_range_t& evRange) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_COMPENSATION_RANGE);
    const size_t ELEM_NUM = sizeof(camera_range_t) / sizeof(int);
    if (entry.count != ELEM_NUM) {
        return NAME_NOT_FOUND;
    }

    evRange.min = entry.data.i32[0];
    evRange.max = entry.data.i32[1];
    return OK;
}

int Parameters::getAeCompensationStep(camera_rational_t& evStep) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_COMPENSATION_STEP);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    evStep.numerator = entry.data.r[0].numerator;
    evStep.denominator = entry.data.r[0].denominator;
    return OK;
}

int Parameters::getSupportedAeExposureTimeRange(std::vector<camera_ae_exposure_time_range_t> & etRanges) const
{
    ParameterHelper::AutoRLock rl(mData);

    const int MEMBER_COUNT = 3;
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_INFO_AE_EXPOSURE_TIME_RANGE);
    if (entry.count == 0 || entry.count % MEMBER_COUNT != 0) {
        return NAME_NOT_FOUND;
    }

    camera_ae_exposure_time_range_t range;
    CLEAR(range);

    for (size_t i = 0; i < entry.count; i += MEMBER_COUNT) {
        range.scene_mode = (camera_scene_mode_t)entry.data.i32[i];
        range.et_range.min = entry.data.i32[i + 1];
        range.et_range.max = entry.data.i32[i + 2];
        etRanges.push_back(range);
    }
    return OK;
}

int Parameters::getSupportedAeGainRange(std::vector<camera_ae_gain_range_t>& gainRanges) const
{
    ParameterHelper::AutoRLock rl(mData);

    const int MEMBER_COUNT = 3;
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_INFO_AE_GAIN_RANGE);
    if (entry.count == 0 || entry.count % MEMBER_COUNT != 0) {
        return NAME_NOT_FOUND;
    }

    camera_ae_gain_range_t range;
    CLEAR(range);

    for (size_t i = 0; i < entry.count; i += MEMBER_COUNT) {
        range.scene_mode = (camera_scene_mode_t)entry.data.i32[i];
        // Since we use int to store float, before storing it we multiply min and max by 100,
        // so we need to divide 100 when giving them outside.
        range.gain_range.min = (float)entry.data.i32[i + 1] / 100.0;
        range.gain_range.max = (float)entry.data.i32[i + 2] / 100.0;
        gainRanges.push_back(range);
    }
    return OK;
}

bool Parameters::getAeLockAvailable() const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AE_LOCK_AVAILABLE);
    if (entry.count != 1) {
        return false;
    }

    return (entry.data.u8[0] == CAMERA_AE_LOCK_AVAILABLE_TRUE);
}

bool Parameters::getAwbLockAvailable() const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_LOCK_AVAILABLE);
    if (entry.count != 1) {
        return false;
    }

    return (entry.data.u8[0] == CAMERA_AWB_LOCK_AVAILABLE_TRUE);
}

int Parameters::setExposureTimeRange(camera_range_t exposureTimeRange)
{
    ParameterHelper::AutoWLock wl(mData);
    const int MEMBER_COUNT = 2;
    int values[MEMBER_COUNT] = {(int)exposureTimeRange.min, (int)exposureTimeRange.max};
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_EXPOSURE_TIME_RANGE, values, MEMBER_COUNT);
}

int Parameters::getExposureTimeRange(camera_range_t& exposureTimeRange) const
{
    ParameterHelper::AutoRLock rl(mData);

    const int MEMBER_COUNT = 2;
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_EXPOSURE_TIME_RANGE);
    if (entry.count == 0 || entry.count != MEMBER_COUNT) {
        return NAME_NOT_FOUND;
    }

    exposureTimeRange.min = entry.data.i32[0];
    exposureTimeRange.max = entry.data.i32[1];
    return OK;
}

int Parameters::setSensitivityGainRange(camera_range_t sensitivityGainRange)
{
    ParameterHelper::AutoWLock wl(mData);
    float values[] = {sensitivityGainRange.min, sensitivityGainRange.max};

    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_SENSITIVITY_GAIN_RANGE, values, ARRAY_SIZE(values));
}

int Parameters::getSensitivityGainRange(camera_range_t& sensitivityGainRange) const
{
    ParameterHelper::AutoRLock rl(mData);

    const int MEMBER_COUNT = 2;
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_SENSITIVITY_GAIN_RANGE);
    if (entry.count == 0 || entry.count != MEMBER_COUNT) {
        return NAME_NOT_FOUND;
    }

    sensitivityGainRange.min = entry.data.f[0];
    sensitivityGainRange.max = entry.data.f[1];
    return OK;
}

int Parameters::setAeConvergeSpeed(camera_converge_speed_t speed)
{
    uint8_t aeSpeed = speed;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_AE_CONVERGE_SPEED, &aeSpeed, 1);
}

int Parameters::getAeConvergeSpeed(camera_converge_speed_t& speed) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_AE_CONVERGE_SPEED);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    speed = (camera_converge_speed_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAwbConvergeSpeed(camera_converge_speed_t speed)
{
    uint8_t awbSpeed = speed;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_CONVERGE_SPEED, &awbSpeed, 1);
}

int Parameters::getAwbConvergeSpeed(camera_converge_speed_t& speed) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_CONVERGE_SPEED);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    speed = (camera_converge_speed_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAeConvergeSpeedMode(camera_converge_speed_mode_t mode)
{
    uint8_t speedMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_AE_CONVERGE_SPEED_MODE, &speedMode, 1);
}

int Parameters::getAeConvergeSpeedMode(camera_converge_speed_mode_t& mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_AE_CONVERGE_SPEED_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    mode = (camera_converge_speed_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAwbConvergeSpeedMode(camera_converge_speed_mode_t mode)
{
    uint8_t speedMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AWB_CONVERGE_SPEED_MODE, &speedMode, 1);
}

int Parameters::getAwbConvergeSpeedMode(camera_converge_speed_mode_t& mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AWB_CONVERGE_SPEED_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    mode = (camera_converge_speed_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setMakernoteData(const void* data, unsigned int size)
{
    CheckError(!data || size == 0, BAD_VALUE, "%s, invalid parameters", __func__);
    ParameterHelper::AutoWLock wl(mData);

    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_MAKERNOTE_DATA, (uint8_t*)data, size);
}

int Parameters::getMakernoteData(void* data, unsigned int* size) const
{
    CheckError(!data || !size, BAD_VALUE, "%s, invalid parameters", __func__);
    ParameterHelper::AutoRLock rl(mData);

    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_MAKERNOTE_DATA);
    if (entry.count > 0) {
        MEMCPY_S(data, *size, entry.data.u8, entry.count);
        *size = entry.count;
    } else {
        return NAME_NOT_FOUND;
    }

    return OK;
}

int Parameters::setCustomAicParam(const void* data, unsigned int length)
{
    CheckError(!data, BAD_VALUE, "%s, invalid parameters", __func__);
    ParameterHelper::AutoWLock wl(mData);

    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_CUSTOM_AIC_PARAM, (uint8_t*)data, length);
}

int Parameters::getCustomAicParam(void* data, unsigned int* length) const
{
    CheckError(!data || !length, BAD_VALUE, "%s, invalid parameters", __func__);
    ParameterHelper::AutoRLock rl(mData);

    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_CUSTOM_AIC_PARAM);
    if (entry.count > 0) {
        MEMCPY_S(data, *length, entry.data.u8, entry.count);
        *length = entry.count;
    } else {
        return NAME_NOT_FOUND;
    }

    return OK;
}

int Parameters::setMakernoteMode(camera_makernote_mode_t mode)
{
    uint8_t mknMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_MAKERNOTE_MODE, &mknMode, 1);
}

int Parameters::getMakernoteMode(camera_makernote_mode_t &mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_MAKERNOTE_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    mode = (camera_makernote_mode_t)entry.data.u8[0];

    return OK;
}

int Parameters::setDigitalZoomRatio(float ratio)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_DIGITAL_ZOOM_RATIO, &ratio, 1);
}

int Parameters::getDigitalZoomRatio(float& ratio) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_DIGITAL_ZOOM_RATIO);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    ratio = entry.data.f[0];
    return OK;
}

int Parameters::setLdcMode(camera_ldc_mode_t mode)
{
    uint8_t ldcMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_LDC_MODE, &ldcMode, 1);
}

int Parameters::getLdcMode(camera_ldc_mode_t &mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_LDC_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_ldc_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setRscMode(camera_rsc_mode_t mode)
{
    uint8_t rscMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_RSC_MODE, &rscMode, 1);
}

int Parameters::getRscMode(camera_rsc_mode_t &mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_RSC_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_rsc_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setFlipMode(camera_flip_mode_t mode)
{
    uint8_t flipMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_FLIP_MODE, &flipMode, 1);
}

int Parameters::getFlipMode(camera_flip_mode_t &mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_FLIP_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_flip_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setMonoDsMode(camera_mono_downscale_mode_t mode)
{
    uint8_t monoDsMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_MONO_DOWNSCALE, &monoDsMode, 1);
}

int Parameters::getMonoDsMode(camera_mono_downscale_mode_t &mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_MONO_DOWNSCALE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_mono_downscale_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setRun3ACadence(int cadence)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_RUN3_A_CADENCE, &cadence, 1);
}

int Parameters::getRun3ACadence(int &cadence) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_RUN3_A_CADENCE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    cadence = entry.data.i32[0];
    return OK;
}

int Parameters::setFisheyeDewarpingMode(camera_fisheye_dewarping_mode_t mode)
{
    uint8_t dewarpingMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_FISHEYE_DEWARPING_MODE, &dewarpingMode, 1);
}

int Parameters::getFisheyeDewarpingMode(camera_fisheye_dewarping_mode_t &mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_FISHEYE_DEWARPING_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_fisheye_dewarping_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAeDistributionPriority(camera_ae_distribution_priority_t priority)
{
    uint8_t distributionPriority = priority;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_AE_DISTRIBUTION_PRIORITY, &distributionPriority, 1);
}

int Parameters::getAeDistributionPriority(camera_ae_distribution_priority_t& priority) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_AE_DISTRIBUTION_PRIORITY);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    priority = (camera_ae_distribution_priority_t)entry.data.u8[0];
    return OK;
}

int Parameters::setYuvColorRangeMode(camera_yuv_color_range_mode_t colorRange)
{
    uint8_t mode = colorRange;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(INTEL_CONTROL_YUV_COLOR_RANGE, &mode, 1);
}

int Parameters::getYuvColorRangeMode(camera_yuv_color_range_mode_t& colorRange) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_CONTROL_YUV_COLOR_RANGE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    colorRange = (camera_yuv_color_range_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setJpegQuality(uint8_t quality)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_QUALITY, &quality, 1);
}

int Parameters::getJpegQuality(uint8_t *quality) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_QUALITY);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    *quality = entry.data.u8[0];
    return OK;
}

int Parameters::setJpegThumbnailQuality(uint8_t quality)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_THUMBNAIL_QUALITY, &quality, 1);
}

int Parameters::getJpegThumbnailQuality(uint8_t *quality) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_THUMBNAIL_QUALITY);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    *quality = entry.data.u8[0];
    return OK;
}

int Parameters::setJpegThumbnailSize(const camera_resolution_t& res)
{
    int size[2] = {res.width, res.height};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_THUMBNAIL_SIZE, size, 2);
}

int Parameters::getJpegThumbnailSize(camera_resolution_t& res) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_THUMBNAIL_SIZE);
    if (entry.count != 2) {
        return NAME_NOT_FOUND;
    }
    res.width  = entry.data.i32[0];
    res.height = entry.data.i32[1];
    return OK;
}

int Parameters::setJpegRotation(int rotation)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_ORIENTATION, &rotation, 1);
}

int Parameters::getJpegRotation(int &rotation) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_ORIENTATION);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    rotation = entry.data.i32[0];
    return OK;
}

int Parameters::setJpegGpsCoordinates(const double *coordinates)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_GPS_COORDINATES, coordinates, 3);
}

int Parameters::getJpegGpsLatitude(double &latitude) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_GPS_COORDINATES);
    if (entry.count != 3) {
        return NAME_NOT_FOUND;
    }
    latitude = entry.data.d[0];
    return OK;
}

int Parameters::getJpegGpsLongitude(double &longitude) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_GPS_COORDINATES);
    if (entry.count != 3) {
        return NAME_NOT_FOUND;
    }
    longitude = entry.data.d[1];
    return OK;
}

int Parameters::getJpegGpsAltitude(double &altitude) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_GPS_COORDINATES);
    if (entry.count != 3) {
        return NAME_NOT_FOUND;
    }
    altitude = entry.data.d[2];
    return OK;
}

int Parameters::setJpegGpsTimeStamp(int64_t  timestamp)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_GPS_TIMESTAMP, &timestamp, 1);
}

int Parameters::getJpegGpsTimeStamp(int64_t &timestamp) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_GPS_TIMESTAMP);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    timestamp = entry.data.i32[0];
    return OK;
}

int Parameters::setJpegGpsProcessingMethod(int processMethod)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_GPS_PROCESSING_METHOD, &processMethod, 1);
}

int Parameters::getJpegGpsProcessingMethod(int &processMethod) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_GPS_PROCESSING_METHOD);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    processMethod = entry.data.u8[0];
    return OK;
}

int Parameters::setJpegGpsProcessingMethod(const char* processMethod)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_JPEG_GPS_PROCESSING_METHOD, (const uint8_t*)processMethod, strlen(processMethod) + 1);
}

int Parameters::getJpegGpsProcessingMethod(int size, char* processMethod) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_JPEG_GPS_PROCESSING_METHOD);
    if (entry.count <= 0) {
        return NAME_NOT_FOUND;
    }
    MEMCPY_S(processMethod, size, entry.data.u8, entry.count);
    return OK;
}

int Parameters::setImageEffect(camera_effect_mode_t  effect)
{
    uint8_t effectmode = effect;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_CONTROL_EFFECT_MODE, &effectmode, 1);
}

int Parameters::getImageEffect(camera_effect_mode_t &effect) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_CONTROL_EFFECT_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    effect = (camera_effect_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setVideoStabilizationMode(camera_video_stabilization_mode_t mode)
{
    uint8_t dvsMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_CONTROL_VIDEO_STABILIZATION_MODE, &dvsMode, 1);
}

int Parameters::getVideoStabilizationMode(camera_video_stabilization_mode_t &mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_CONTROL_VIDEO_STABILIZATION_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_video_stabilization_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::getFocalLength(float &focal) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_FOCAL_LENGTH);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    focal = (float)entry.data.f[0];
    return OK;
}

int Parameters::setFocalLength(float focal)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_LENS_FOCAL_LENGTH, &focal, 1);
}

int Parameters::getAperture(float &aperture) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_APERTURE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    aperture = entry.data.f[0];
    return OK;
}

int Parameters::setAperture(float aperture)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_LENS_APERTURE, &aperture, 1);
}

int Parameters::getFocusDistance(float &distance) const {
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_FOCUS_DISTANCE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    distance = entry.data.f[0];
    return OK;
}

int Parameters::setFocusDistance(float distance) {
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_LENS_FOCUS_DISTANCE, &distance, 1);
}

int Parameters::setFocusRange(const camera_range_t &focusRange)
{
    float range[] = {focusRange.min, focusRange.max};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_LENS_FOCUS_RANGE,
                                                      range, ARRAY_SIZE(range));
}

int Parameters::getFocusRange(camera_range_t& focusRange) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_FOCUS_RANGE);
    if (entry.count != (sizeof(camera_range_t) / sizeof(float))) {
        return NAME_NOT_FOUND;
    }
    focusRange.min = entry.data.f[0];
    focusRange.max = entry.data.f[1];
    return OK;
}

int Parameters::setAfMode(camera_af_mode_t afMode)
{
    uint8_t mode = afMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AF_MODE, &mode, 1);
}

int Parameters::getAfMode(camera_af_mode_t& afMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AF_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    afMode = (camera_af_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAfTrigger(camera_af_trigger_t afTrigger)
{
    uint8_t trigger = afTrigger;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AF_TRIGGER, &trigger, 1);
}

int Parameters::getAfTrigger(camera_af_trigger_t& afTrigger) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AF_TRIGGER);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    afTrigger = (camera_af_trigger_t)entry.data.u8[0];
    return OK;
}

int Parameters::setAfRegions(camera_window_list_t afRegions)
{
    ParameterHelper::AutoWLock wl(mData);
    return setRegions(ParameterHelper::getMetadata(mData), afRegions, CAMERA_AF_REGIONS);
}

int Parameters::getAfRegions(camera_window_list_t& afRegions) const
{
    ParameterHelper::AutoRLock rl(mData);
    return getRegions(ParameterHelper::getMetadataEntry(mData, CAMERA_AF_REGIONS), afRegions);
}

int Parameters::setAfState(camera_af_state_t afState)
{
    uint8_t state = afState;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_AF_STATE, &state, 1);
}

int Parameters::getAfState(camera_af_state_t& afState) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_AF_STATE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    afState = (camera_af_state_t)entry.data.u8[0];
    return OK;
}

int Parameters::setLensState(bool lensMoving)
{
    uint8_t state = (lensMoving) ? 1 : 0;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_LENS_STATE, &state, 1);
}

int Parameters::getLensState(bool& lensMoving) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_STATE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    lensMoving = (entry.data.u8[0] > 0);
    return OK;
}

int Parameters::getLensAperture(float &aperture) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_INFO_AVAILABLE_APERTURES);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    aperture = entry.data.f[0];
    return OK;
}

int Parameters::getLensFilterDensity(float &filterDensity) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData,
                                                   CAMERA_LENS_INFO_AVAILABLE_FILTER_DENSITIES);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    filterDensity = entry.data.f[0];
    return OK;
}

int Parameters::getLensMinFocusDistance(float &minFocusDistance) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_INFO_MINIMUM_FOCUS_DISTANCE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    minFocusDistance = entry.data.f[0];
    return OK;
}

int Parameters::getLensHyperfocalDistance(float &hyperfocalDistance) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_INFO_HYPERFOCAL_DISTANCE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    hyperfocalDistance = entry.data.f[0];
    return OK;
}

int Parameters::getSensorMountType(camera_mount_type_t& sensorMountType) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, INTEL_INFO_SENSOR_MOUNT_TYPE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    sensorMountType = (camera_mount_type_t)entry.data.u8[0];
    return OK;
}

// User can set envrionment and then call api to update the debug level.
int Parameters::updateDebugLevel()
{
    Log::setDebugLevel();
    CameraDump::setDumpLevel();
    return OK;
}

int Parameters::setTestPatternMode(camera_test_pattern_mode_t mode)
{
    int32_t testPatterMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_SENSOR_TEST_PATTERN_MODE, &testPatterMode, 1);
}

int Parameters::getTestPatternMode(camera_test_pattern_mode_t& mode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SENSOR_TEST_PATTERN_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_test_pattern_mode_t)entry.data.i32[0];
    return OK;
}

int Parameters::setCropRegion(camera_crop_region_t cropRegion)
{
    int values[] = {cropRegion.flag, cropRegion.x, cropRegion.y};
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_SCALER_CROP_REGION, values, ARRAY_SIZE(values));
}

int Parameters::getCropRegion(camera_crop_region_t& cropRegion) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SCALER_CROP_REGION);
    if (entry.count <= 0) {
        return NAME_NOT_FOUND;
    }
    cropRegion.flag = entry.data.i32[0];
    cropRegion.x = entry.data.i32[1];
    cropRegion.y = entry.data.i32[2];
    return OK;
}

int Parameters::setControlSceneMode(uint8_t sceneModeValue)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_CONTROL_SCENE_MODE, &sceneModeValue, 1);
}

int Parameters::setFaceDetectMode(uint8_t faceDetectMode)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_STATISTICS_FACE_DETECT_MODE, &faceDetectMode, 1);
}

int Parameters::getFaceDetectMode(uint8_t& faceDetectMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_STATISTICS_FACE_DETECT_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    faceDetectMode = entry.data.u8[0];
    return OK;
}

int Parameters::setFaceIds(int *faceIds, int faceNum)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_STATISTICS_FACE_IDS, faceIds, faceNum);
}

int Parameters::getSensorActiveArraySize(camera_coordinate_system_t& arraySize) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SENSOR_INFO_ACTIVE_ARRAY_SIZE);
    if (entry.count <= 0) {
        return NAME_NOT_FOUND;
    }
    arraySize.left = entry.data.i32[0];
    arraySize.top = entry.data.i32[1];
    arraySize.right = arraySize.left + entry.data.i32[2]; //width
    arraySize.bottom = arraySize.top + entry.data.i32[3]; //height
    return OK;
}

int Parameters::setShadingMode(camera_shading_mode_t shadingMode)
{
    uint8_t mode = shadingMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_SHADING_MODE, &mode, 1);
}

int Parameters::getShadingMode(camera_shading_mode_t& shadingMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_SHADING_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    shadingMode = (camera_shading_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::setLensShadingMapMode(camera_lens_shading_map_mode_type_t lensShadingMapMode)
{
    uint8_t mode = lensShadingMapMode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_STATISTICS_LENS_SHADING_MAP_MODE,
                                                      &mode, 1);
}

int Parameters::getLensShadingMapMode(
                                  camera_lens_shading_map_mode_type_t &lensShadingMapMode) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_STATISTICS_LENS_SHADING_MAP_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    lensShadingMapMode = (camera_lens_shading_map_mode_type_t)entry.data.u8[0];
    return OK;
}

int Parameters::setLensShadingMap(const float *lensShadingMap, size_t lensShadingMapSize)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_STATISTICS_LENS_SHADING_MAP,
                                                      lensShadingMap, lensShadingMapSize);
}

int Parameters::getLensShadingMap(float **lensShadingMap, size_t &lensShadingMapSize) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_STATISTICS_LENS_SHADING_MAP);

    if (entry.count <= 0 || !lensShadingMap) {
        return NAME_NOT_FOUND;
    }

    *lensShadingMap = const_cast<float*>(entry.data.f);
    lensShadingMapSize = entry.count;
    return OK;
}

int Parameters::getLensInfoShadingMapSize(camera_coordinate_t &shadingMapSize) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_LENS_INFO_SHADING_MAP_SIZE);
    if (entry.count <= 0) {
        return NAME_NOT_FOUND;
    }

    shadingMapSize.x = entry.data.i32[0];  // width
    shadingMapSize.y = entry.data.i32[1];  // height
    return OK;
}

int Parameters::setTonemapMode(camera_tonemap_mode_t mode) {
    uint8_t tMode = mode;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_TONEMAP_MODE, &tMode, 1);
}

int Parameters::getTonemapMode(camera_tonemap_mode_t& mode) const {
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_MODE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    mode = (camera_tonemap_mode_t)entry.data.u8[0];
    return OK;
}

int Parameters::getSupportedTonemapMode(vector<camera_tonemap_mode_t>& tonemapModes) const {
    ParameterHelper::AutoRLock rl(mData);

    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_AVAILABLE_TONE_MAP_MODES);
    for (size_t i = 0; i < entry.count; i++) {
        tonemapModes.push_back((camera_tonemap_mode_t)entry.data.u8[i]);
    }
    return OK;
}

int Parameters::setTonemapPresetCurve(camera_tonemap_preset_curve_t type) {
    uint8_t cType = type;
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_TONEMAP_PRESET_CURVE, &cType, 1);
}

int Parameters::getTonemapPresetCurve(camera_tonemap_preset_curve_t& type) const {
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_PRESET_CURVE);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    type = (camera_tonemap_preset_curve_t)entry.data.u8[0];
    return OK;
}

int Parameters::setTonemapGamma(float gamma) {
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_TONEMAP_GAMMA, &gamma, 1);
}

int Parameters::getTonemapGamma(float& gamma) const {
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_GAMMA);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    gamma = entry.data.f[0];
    return OK;
}

int Parameters::getTonemapMaxCurvePoints(int32_t& number) const {
    ParameterHelper::AutoRLock rl(mData);

    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_MAX_CURVE_POINTS);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    number = entry.data.i32[0];
    return OK;
}

int Parameters::setTonemapCurves(const camera_tonemap_curves_t& curves) {
    ParameterHelper::AutoWLock wl(mData);
    int ret = ParameterHelper::getMetadata(mData).update(CAMERA_TONEMAP_CURVE_RED,
                                                         curves.rCurve, curves.rSize);
    ret |= ParameterHelper::getMetadata(mData).update(CAMERA_TONEMAP_CURVE_BLUE,
                                                      curves.bCurve, curves.bSize);
    ret |= ParameterHelper::getMetadata(mData).update(CAMERA_TONEMAP_CURVE_GREEN,
                                                      curves.gCurve, curves.gSize);
    return ret;
}

int Parameters::getTonemapCurves(camera_tonemap_curves_t& curves) const {
    curves.rSize = 0;
    curves.bSize = 0;
    curves.gSize = 0;

    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_CURVE_RED);
    if (entry.count > 0) {
        curves.rSize = entry.count;
        curves.rCurve = entry.data.f;
    }
    entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_CURVE_BLUE);
    if (entry.count > 0) {
        curves.bSize = entry.count;
        curves.bCurve = entry.data.f;
    }
    entry = ParameterHelper::getMetadataEntry(mData, CAMERA_TONEMAP_CURVE_GREEN);
    if (entry.count > 0) {
        curves.gSize = entry.count;
        curves.gCurve = entry.data.f;
    }
    return (curves.rSize && curves.bSize && curves.gSize) ? OK : NAME_NOT_FOUND;
}

int Parameters::setUserRequestId(int32_t userRequestId) {
    ParameterHelper::AutoWLock wl(mData);

    return ParameterHelper::getMetadata(mData).update(CAMERA_REQUEST_ID, &userRequestId, 1);
}

int Parameters::getUserRequestId(int32_t& userRequestId) const {
    ParameterHelper::AutoRLock rl(mData);

    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_REQUEST_ID);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }

    userRequestId = entry.data.i32[0];
    return OK;
}

int Parameters::setCaptureIntent(uint8_t captureIntent)
{
    ParameterHelper::AutoWLock wl(mData);
    return ParameterHelper::getMetadata(mData).update(CAMERA_CONTROL_CAPTUREINTENT,
                                                      &captureIntent, 1);
}

int Parameters::getCaptureIntent(uint8_t& captureIntent) const
{
    ParameterHelper::AutoRLock rl(mData);
    auto entry = ParameterHelper::getMetadataEntry(mData, CAMERA_CONTROL_CAPTUREINTENT);
    if (entry.count != 1) {
        return NAME_NOT_FOUND;
    }
    captureIntent = entry.data.u8[0];
    return OK;
}

} // end of namespace icamera
