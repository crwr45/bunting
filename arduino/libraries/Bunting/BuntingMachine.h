#include "BasicStepperDriver.h"


class BuntingMachine
{
  public:
    BuntingMachine(int dir, short steps, short dir_pin, short step_pin, 
                   short enable_pin, short stopperPin, int deployDegrees, 
                   short rpm, short microsteps);
    BuntingMachine(BasicStepperDriver stepper, int dir, short stopperPin, 
                   int deployDegrees, short rpm, short microsteps);

    static const int DEPLOYING = 0;
    static const int STORING = 1;

    static const int STORED = -1;
    static const int UNKNOWN = 0;
    static const int DEPLOYED = 1;

    void setup(void (*ISR)(void));
    
    void _reverse();

    void stop();
    void store();
    void deploy();
    void enableMotor();
    void disableMotor();

  private:
    BasicStepperDriver _stepperDriver;
    short _stopperPin;
    int _dir_multi;
    int _direction;
    int _state;
    int _deployDegrees;
};
