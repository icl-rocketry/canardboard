#pragma once

#include "librrc/nrcremoteactuatorbase.h"
#include "librrc/nrcremoteservo.h"

#include <librnp/rnp_networkmanager.h>
#include <librnp/rnp_packet.h>
#include <librrc/packets/servocalibrationpacket.h>
#include "SiC43x.h"
#include "sensors/icm_20608.h"
#include "sensors/sensorStructs.h"

struct GyroReading
{
    float rollRate;
    uint32_t timestamp;

    GyroReading(float rollRate, uint32_t timestamp):
        rollRate(rollRate), 
        timestamp(timestamp)
        {};
};


class GyroBuf : private std::queue<GyroReading> {
    public:
        /**
         * @brief inserts a new value at the front and returns the removed value. 
         * Returns zero if len != maxLen
         * 
         * @param val 
         * @return T 
         */
        GyroReading pop_push_back(GyroReading val){
            // push new element on queue
            this->push(val);
            
            const size_t curr_size = this->size();


            GyroReading lastVal = this->front();
            if(val.timestamp - lastVal.timestamp > rollingAverageDuration){
                
            }
            while (val.timestamp - lastVal.timestamp > rollingAverageDuration)
            {
                this->pop();
            }
                return lastVal;
            } else{
                return GyroReading(0,0);
            }



            if (curr_size == maxLen + 1){
                
                this->pop();
                return lastVal;
            }else 
            if (curr_size < maxLen + 1) {
                return GyroReading(0,0);
            }else
            if (curr_size > maxLen + 1){
                throw std::runtime_error("GyroBuf size exceeded!");
            }
            return GyroReading(0,0);

        };

        size_t size(){
            return this->size();
        }

        void push(GyroReading val){
            this->push(val);
        }

        using std::queue<GyroReading>::size;

    private:
        static constexpr size_t maxLen = 1000;
        static constexpr uint32_t rollingAverageDuration = 50;


};

class NRCGeddan : public NRCRemoteActuatorBase<NRCGeddan>
{

    public:

        NRCGeddan(RnpNetworkManager &networkmanager,
                    SiC43x &buck,
                    ICM_20608 &imu,
                    uint8_t geddanServo1GPIO,
                    uint8_t geddanServo1Channel,
                    uint8_t geddanServo2GPIO,
                    uint8_t geddanServo2Channel,
                    uint8_t geddanServo3GPIO,
                    uint8_t geddanServo3Channel,
                    uint8_t address
                    ):
            NRCRemoteActuatorBase(networkmanager),
            _networkmanager(networkmanager),
            _buck(buck),
            _imu(imu),
            _geddanServo1GPIO(geddanServo1GPIO),
            _geddanServo1Channel(geddanServo1Channel),
            _geddanServo2GPIO(geddanServo2GPIO),
            _geddanServo2Channel(geddanServo2Channel),
            _geddanServo3GPIO(geddanServo3GPIO),
            _geddanServo3Channel(geddanServo3Channel),
            _address(address),
            geddanServo1(geddanServo1GPIO,geddanServo1Channel,networkmanager),
            geddanServo2(geddanServo2GPIO,geddanServo2Channel,networkmanager),
            geddanServo3(geddanServo3GPIO,geddanServo3Channel,networkmanager)
            {};
        

        void setup();
        void update();

        

    protected:

        RnpNetworkManager& _networkmanager;
        SiC43x& _buck;
        ICM_20608& _imu;
        const uint8_t _geddanServo1GPIO;
        const uint8_t _geddanServo1Channel;
        const uint8_t _geddanServo2GPIO;
        const uint8_t _geddanServo2Channel;
        const uint8_t _geddanServo3GPIO;
        const uint8_t _geddanServo3Channel;
        const uint8_t _address;

        NRCRemoteServo geddanServo1;
        NRCRemoteServo geddanServo2;    
        NRCRemoteServo geddanServo3;  
        
        friend class NRCRemoteActuatorBase;
        friend class NRCRemoteBase;

        uint64_t prevLogMessageTime;
        //Calibration

        void loadCalibration();
        float _default_angle1 = 45;
        float _default_angle2 = 45;
        float _default_angle3 = 45;

        void setHome(float homeangle1, float homeangle2, float homeangle3){
            _default_angle1 = homeangle1;
            _default_angle2 = homeangle2;
            _default_angle3 = homeangle3;
        };

        //Control
        
        void allGotoRawAngle(float angle);
        void allGotoCalibratedAngle(float angle);
        void updateTargetRollRate(float targetRollRate);

        void execute_impl(packetptr_t packetptr);
        void calibrate_impl(packetptr_t packetptr);
        //void arm_impl(packetptr_t packetptr);
        //void disarm_impl(packetptr_t packetptr);
        void override_impl(packetptr_t packetptr);
        void extendedCommandHandler_impl(const NRCPacket::NRC_COMMAND_ID commandID,packetptr_t packetptr);

        enum class GeddanState : uint8_t
        {
            ConstantRoll = 0,
            HoldZero = 1,
            WiggleTest = 2,
            Debug = 3, 
            Fun = 4,
        };

        GeddanState currentGeddanState = GeddanState::ConstantRoll;

        //P Controller

        void resetMovingAverage();

        float error;
        const float _kp = 0.05;
        SensorStructs::ACCELGYRO_6AXIS_t _imudata;

        float _zRollRate;
        float _targetRollRate;

        GyroBuf gyroBuf;
        float rollingAverageSum;
        float rollingAverage;
        

        
        //Wiggle Test Stuff

        const uint64_t zeroCanards = 0;
        // const uint64_t startSlowSpinLeft = 1000;
        // const uint64_t zeroCanards2 = 2000;
        // const uint64_t startSlowSpinRight = 3000;
        // const uint64_t zeroCanards3 = 4000;
        // const uint64_t zeroCanards4 = 5000;
        // const uint64_t startSpinLeft = 5500;
        // const uint64_t startSpinRight = 6000;
        // const uint64_t endOfWiggleSeq = 6500;

        const uint64_t startSlowSpinLeft = 100;
        const uint64_t zeroCanards2 = 200;
        const uint64_t startSlowSpinRight = 300;
        const uint64_t zeroCanards3 = 400;
        const uint64_t zeroCanards4 = 500;
        const uint64_t startSpinLeft = 550;
        const uint64_t startSpinRight = 600;
        const uint64_t endOfWiggleSeq = 650;
        float lerp(float x, float in_min, float in_max, float out_min, float out_max);
        bool timeFrameCheck(int64_t start_time, int64_t end_time = -1);

        uint32_t wiggleTestTime;
        GeddanState previousGeddanState;

        void logReadings();
        uint32_t telemetry_log_delta = 5000;
        uint32_t prev_telemetry_log_time;
};