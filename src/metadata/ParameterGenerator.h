/*
 * Copyright (C) 2015-2023 Intel Corporation.
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

#pragma once

#include <map>
#include <memory>

#include "Parameters.h"
#include "iutils/Thread.h"

namespace icamera {

class RequestParam {
 public:
    RequestParam() : requestId(-1) {}

    ~RequestParam() {}

    long requestId;
    Parameters param;

 private:
    RequestParam(const RequestParam& other);
    RequestParam& operator=(const RequestParam& other);
};

/*
 * \class ParameterGenerator
 * This class is used to generator parameter results. It updates the parameters
 * with AIQ results, sensor embedded metadata and 3A statistics.
 * The parameter results are stored with the frame sequence indicating on which
 * frame the parameters are active.
 */
class ParameterGenerator {
 public:
    ParameterGenerator(int cameraId);
    ~ParameterGenerator();

    /**
     * \brief reset the parameters data.
     */
    int reset();

    /**
     * \brief Save parameters with sequence id indicating the active frame.
     *           And update the aiq result parameters as well.
     */
    int saveParameters(int64_t sequence, long requestId,
                       std::shared_ptr<RequestParam> requestParam = nullptr);
    std::shared_ptr<RequestParam> getRequestParamBuf();

    /**
     * \brief Update parameters per sequence id.
     */
    void updateParameters(int64_t sequence, const Parameters* param);
    int getIspParameters(int64_t sequence, Parameters* param);
    int getRawOutputMode(int64_t sequence, raw_data_output_t& rawOutputMode);
    int getUserRequestId(int64_t sequence, int32_t& userRequestId);

    /**
     * \brief Get the parameters for the frame indicated by the sequence id.
     */
    int getParameters(int64_t sequence, Parameters* param, bool setting = true, bool result = true);
    int getRequestId(int64_t predictSequence, long& requestId);

 private:
    ParameterGenerator(const ParameterGenerator& other);
    ParameterGenerator& operator=(const ParameterGenerator& other);

    int generateParametersL(int64_t sequence, Parameters* params);
    int updateWithAiqResultsL(int64_t sequence, Parameters* params);
    int updateAwbGainsL(Parameters* params, const cca::cca_awb_results& result);
    int updateCcmL(Parameters* params, const AiqResult* aiqResult);
    int updateTonemapCurve(int64_t sequence, Parameters* params);

    int updateCommonMetadata(Parameters* params, const AiqResult* aiqResult);

 private:
    int mCameraId;
    static const int kStorageSize = MAX_SETTING_COUNT;

    // Guard for ParameterGenerator public API.
    Mutex mParamsLock;
    // first: sequence id, second: RequestParam data
    std::map<int64_t, std::shared_ptr<RequestParam> > mRequestParamMap;

    std::unique_ptr<float[]> mTonemapCurveRed;
    std::unique_ptr<float[]> mTonemapCurveBlue;
    std::unique_ptr<float[]> mTonemapCurveGreen;
    int32_t mTonemapMaxCurvePoints;

    camera_color_transform_t mPaCcm;
};

} /* namespace icamera */
