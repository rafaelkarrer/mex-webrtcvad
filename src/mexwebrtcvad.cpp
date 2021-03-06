/*
* mwebrtcvad.cpp - MATLAB WebRTC VAD interface
*
* Provides an interface for WebRTC's VAD in MATLAB
*
* Rafael Karrer, 31.07.2020
*
*/

#include "mex.hpp"
#include "mexAdapter.hpp"

#include "webrtc\common_audio\vad\include\webrtc_vad.h"

class MexFunction : public matlab::mex::Function {
    std::ostringstream stream;
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
    matlab::data::ArrayFactory factory;
    int vadResult;
    
    VadInst* vadHandle;
    
public:
    MexFunction() {
        vadHandle = NULL;
    }
    
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {

        checkArguments(outputs, inputs);

        matlab::data::CharArray cmdCharArray = inputs[0];
        
        if (cmdCharArray.toAscii() == factory.createCharArray("Init").toAscii()) {
            vadInit();
            outputs[0] = factory.createArray<double>({ 0,0 }, { });
            return;
        } else if (cmdCharArray.toAscii() == factory.createCharArray("SetMode").toAscii()) {
            vadSetMode( (int)inputs[1][0] );
            //stream << "vad mode = " << (int)inputs[1][0] << std::endl;
            //writeToConsole(stream);
            outputs[0] = factory.createArray<double>({ 0,0 }, { });
            return;
        } else if (cmdCharArray.toAscii() == factory.createCharArray("Process").toAscii()) {
            matlab::data::TypedArray<int16_t> speech = std::move(inputs[2]);

            int16_t *frame = speech.release().get();
            vadResult = vadProcess( (int)inputs[1][0], frame, (int)inputs[3][0] );
            //stream << "VAD=" << vad << (int)inputs[1][0] << (int)inputs[3][0] << frame[0] << frame[1] << std::endl;
            //writeToConsole(stream);
            
            outputs[0] = factory.createArray<double>({ 1,1 }, { (double)vadResult });
            return;
            
        } else if (cmdCharArray.toAscii() == factory.createCharArray("Free").toAscii()) {
            vadFree();
            outputs[0] = factory.createArray<double>({ 0,0 }, { });
            return;
        }
        //matlab::data::TypedArray<double> in = std::move(0);
        
        outputs[0] = factory.createArray<double>({ 0,0 }, { });
    }

    void vadInit() {
        if( vadHandle != NULL ) {
            WebRtcVad_Free(vadHandle);
        }
        vadHandle = WebRtcVad_Create();
        WebRtcVad_Init(vadHandle);
    }

    void vadSetMode( int mode ) {
        if( vadHandle == NULL ) {
            WebRtcVad_Init(vadHandle);
        }
        WebRtcVad_set_mode(vadHandle, mode);
    }
    
    int vadProcess( int fs, const int16_t* audio_frame, size_t frame_length ) {
        if( vadHandle != NULL ) {
            return WebRtcVad_Process(vadHandle, fs, audio_frame, frame_length);
        } else {
            return -1;
        }
    }
    
    void vadFree() {
        if( vadHandle != NULL ) {
            WebRtcVad_Free(vadHandle);
        }
        vadHandle = NULL;
    }
        
    void arrayProduct(matlab::data::TypedArray<double>& inMatrix, double multiplier) {
        
        for (auto& elem : inMatrix) {
            elem *= multiplier;
        }
    }

    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        if (inputs[0].getType() != matlab::data::ArrayType::CHAR) {
            matlabPtr->feval(u"error", 
                0, std::vector<matlab::data::Array>({ factory.createScalar("Action argument must be one of the following strings: 'Init', 'SetMode', 'Process', 'Free'") }));            
        }

        matlab::data::CharArray cmdCharArray = inputs[0];
        
        if (cmdCharArray.toAscii() == factory.createCharArray("Init").toAscii()) {
            // nothing to check
        } else if (cmdCharArray.toAscii() == factory.createCharArray("SetMode").toAscii()) {
            if (inputs.size() != 2) {
                matlabPtr->feval(u"error", 
                    0, std::vector<matlab::data::Array>({ factory.createScalar("Action 'Init' requires 1 argument: vad mode/aggressiveness (<double>)") }));  
            }
        } else if (cmdCharArray.toAscii() == factory.createCharArray("Process").toAscii()) {
            if (inputs.size() != 4) {
                matlabPtr->feval(u"error", 
                    0, std::vector<matlab::data::Array>({ factory.createScalar("Action 'Process' requires 3 arguments: sample rate (<double>), audio_frame (<int16> array), frame_length (<double>)") }));  
            }
            if (inputs[1].getType() != matlab::data::ArrayType::DOUBLE) {
                matlabPtr->feval(u"error", 
                    0, std::vector<matlab::data::Array>({ factory.createScalar("Argument 1 (sample rate) must be of type <double>") }));            
            }
            if (inputs[2].getType() != matlab::data::ArrayType::INT16) {
                matlabPtr->feval(u"error", 
                    0, std::vector<matlab::data::Array>({ factory.createScalar("Argument 2 (audio_frame) must be of type <int16>") }));            
            }
            if (inputs[3].getType() != matlab::data::ArrayType::DOUBLE) {
                matlabPtr->feval(u"error", 
                    0, std::vector<matlab::data::Array>({ factory.createScalar("Argument 3 (frame_length) must be of type <double>") }));            
            }
        
        } else if (cmdCharArray.toAscii() == factory.createCharArray("Free").toAscii()) {
            // nothing to check
        } else {
            matlabPtr->feval(u"error", 
                0, std::vector<matlab::data::Array>({ factory.createScalar("Action argument must be one of the following strings: 'Init', 'SetMode', 'Process'") })); 
        }
        
//         if (inputs.size() != 2) {
//             matlabPtr->feval(u"error", 
//                 0, std::vector<matlab::data::Array>({ factory.createScalar("Two inputs required") }));
//         }
// 
//         if (inputs[0].getNumberOfElements() != 1) {
//             matlabPtr->feval(u"error", 
//                 0, std::vector<matlab::data::Array>({ factory.createScalar("Input multiplier must be a scalar") }));
//         }
//         
//         if (inputs[0].getType() != matlab::data::ArrayType::DOUBLE ||
//             inputs[0].getType() == matlab::data::ArrayType::COMPLEX_DOUBLE) {
//             matlabPtr->feval(u"error", 
//                 0, std::vector<matlab::data::Array>({ factory.createScalar("Input multiplier must be a noncomplex scalar double") }));
//         }
// 
//         if (inputs[1].getType() != matlab::data::ArrayType::DOUBLE ||
//             inputs[1].getType() == matlab::data::ArrayType::COMPLEX_DOUBLE) {
//             matlabPtr->feval(u"error", 
//                 0, std::vector<matlab::data::Array>({ factory.createScalar("Input matrix must be type double") }));
//         }
// 
//         if (inputs[1].getDimensions().size() != 2) {
//             matlabPtr->feval(u"error", 
//                 0, std::vector<matlab::data::Array>({ factory.createScalar("Input must be m-by-n dimension") }));
//         }
    }
    
    void writeToConsole(std::ostringstream& stream) {
        // Pass stream content to MATLAB fprintf function
        matlabPtr->feval(u"fprintf", 0,
            std::vector<matlab::data::Array>({ factory.createScalar(stream.str()) }));
        // Clear stream buffer
        stream.str("");
    }
};