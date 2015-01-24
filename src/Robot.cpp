#include "Robot.hpp"
#include <cmath>

Robot::Robot() : settings( "/home/lvuser/RobotSettings.txt" ) ,
                 drive1Buttons( 0 ) ,
                 drive2Buttons( 1 ) ,
                 elevatorButtons( 2 ) ,
                 dsDisplay( DriverStationDisplay::getInstance(
                     settings.getInt( "DS_Port" ) ) ) ,
                 pidGraph( 3513 ) {

	std::cout << "Constructor" << std::endl;

    robotDrive = new DriveTrain();

    driveStick1 = new Joystick( 0 );
    driveStick2 = new Joystick( 1 );
    shootStick = new Joystick( 2 );
    autonTimer = new Timer();
    displayTimer = new Timer();
    ev = new Elevator();


    /* dsDisplay.addAutonMethod( "DriveForward Autonomous" ,
                                   &Robot::DriveForwardAuton ,
                                   this );
    dsDisplay.addAutonMethod( "Right/Left Autonomous" ,
                                   &Robot::RightLeftAuton ,
                                   this );
    dsDisplay.addAutonMethod( "MotionProfile" ,
                                   &Robot::AutonMotionProfile ,
                                   this );
    dsDisplay.addAutonMethod( "Side Auton" , &Robot::SideAuton , this ); */
    dsDisplay.addAutonMethod( "Noop Auton" , &Robot::NoopAuton , this );

    pidGraph.setSendInterval( 200 );

    insight = Insight::getInstance( settings.getInt( "Insight_Port" ) );

    logger1 = new Logger();
    ls = new LogStream( logger1 );
    logFileSink = new LogFileSink( "/home/admin/LogFile.txt" );
    logServerSink = new LogServerSink();
    logger1->addLogSink( logFileSink );
    logger1->addLogSink( logServerSink );
    logFileSink->setVerbosityLevels( LogEvent::VERBOSE_ALL );
    logServerSink->setVerbosityLevels( LogEvent::VERBOSE_ALL );
    logServerSink->startServer( 4097 );
    displayTimer->Start();
}

Robot::~Robot() {
    delete robotDrive;

    delete driveStick1;
    delete driveStick2;
    delete shootStick;

    delete autonTimer;
    delete displayTimer;
    delete ev;
    delete logger1;
    delete ls;
    delete logFileSink;
    delete logServerSink;
}
void Robot::calibrateTalons() {
    robotDrive->drive( 1 , 0 );
    Wait( 3.0 );
    robotDrive->drive( 0 , 0 );
    Wait( 3.0 );
    robotDrive->drive( -1 , 0 );
    Wait( 3.0 );
    robotDrive->drive( 0 , 0 );
    Wait( 3.0 );
}

void Robot::OperatorControl() {

    robotDrive->reloadPID();

        // DS_PrintOut();

        // arcade Drive
        if ( driveStick2->GetRawButton( 2 ) ) {
            robotDrive->drive( driveStick1->GetY() , driveStick2->GetZ() ,
                               true );
        }
        else {
            robotDrive->drive( driveStick1->GetY() , driveStick2->GetZ() );
        }

        /*
        CurrentState = ds->GetStickButton(0,1);

        		if(CurrentState != LastState && CurrentState == true){
        			std::cout << ev->;
        			ev->(!ev->);
        		}
        		LastState = CurrentState;

        		ds->GetStickButton(1,1);

        		shootStick->GetTrigger();
		*/

        /* Trailing edge of trigger press */
        if(elevatorButtons.releasedButton(1)) {
        	// ...
        	std::cout << ev->getElevatorGrab();
        	ev->elevatorGrab(!ev->getElevatorGrab());
        }
        if(elevatorButtons.releasedButton(2)){
        	std::cout << ev->getIntakeGrab();
        	ev->intakeGrab(!ev->getIntakeGrab());
    	}
    	if(elevatorButtons.releasedButton(3)){
    		std::cout << ev->getIntakeVer();
    		ev->intakeVer(!ev->getIntakeVer());
    	}
    	if(elevatorButtons.releasedButton(4)){
    		std::cout << ev->getIntakeWheels();
    		ev->intakeWheels(Elevator::S_FORWARD);
    	}else if(elevatorButtons.releasedButton(5)){
    		std::cout << ev->getIntakeWheels();
    		ev->intakeWheels(Elevator::S_REVERSED);
    	}else if(elevatorButtons.releasedButton(0)){
    		std::cout << ev->getIntakeWheels();
    		ev->intakeWheels(Elevator::S_STOPPED);
    	}

		drive1Buttons.updateButtons();
		drive2Buttons.updateButtons();
		elevatorButtons.updateButtons();

		DS_PrintOut();

		Wait( 0.01 );

    }


void Robot::Autonomous() {
    autonTimer->Reset();
    autonTimer->Start();

    dsDisplay.execAutonomous();

    autonTimer->Stop();
}

void Robot::Disabled() {


  //  while ( IsDisabled() ) {


    //    DS_PrintOut();

      //  logServerSink->acceptor( false );
        // Wait( 0.1 );
  //  }
	}

void Robot::Test() {


    /* calibrateTalons();
     *
     *  mainCompressor->Start();
     *
     *  testDriveTrain(true, true, -1, 1);
     *  testDriveTrain(true,false, -1, 1);
     *  testDriveTrain(false, true, -1, 1);
     *  testDriveTrain(false, false, -1, 1);
     *  testCompressor();
     *  robotDrive->drive( 0 , 0 ); */
}

bool Robot::testDriveTrain( bool shifterState ,
                            bool direction ,
                            float lowerBound ,
                            float upperBound ) {
    Timer timer;
    timer.Start();

    // Converts direction (1 or 0) to 1 or -1 respectively
    int i = static_cast<int>( direction ) * 2 - 1;

    robotDrive->setGear( shifterState );

    while ( !timer.HasPeriodPassed( 3.0 ) ) {
        robotDrive->drive( i , 0 );
        Wait( 0.1 );
        if ( !( lowerBound < robotDrive->getLeftRate() &&
                robotDrive->getLeftRate() < upperBound ) ) {
            return false;
        }
    }

    return true;
}

void Robot::DS_PrintOut() {
    if ( pidGraph.hasIntervalPassed() ) {
        pidGraph.graphData( robotDrive->getLeftDist() , "Left PID" );
        pidGraph.graphData( robotDrive->getLeftSetpoint() , "Left Setpoint" );


        pidGraph.resetInterval();
    }

    if ( displayTimer->HasPeriodPassed( 0.5 ) ) {
        // (*ls) << SetLogLevel(LogEvent::VERBOSE_INFO) << kinect->GetArmScale().second << std::flush;
        // logServerSink->acceptor(false);

        // DriverStationLCD *userMessages = DriverStationLCD::GetInstance();
        // userMessages->Clear();

        // userMessages->Printf(DriverStationLCD::kUser_Line1, 1,"accelerometer %f ",accelerometer->GetAcceleration(ADXL345_I2C_ALT::kAxis_X));

        // userMessages->Printf(DriverStationLCD::kUser_Line2, 1,"Encoder2: %f",robotPosition->GetLeftEncoder());
        // userMessages->Printf(DriverStationLCD::kUser_Line3, 1," Left : %f", (driveStick1->GetTwist() + 1)/2);
        // userMessages->Printf(DriverStationLCD::kUser_Line4, 1," Right : %f", (driveStick2->GetTwist() + 1)/2 );

        // userMessages->UpdateLCD();

        dsDisplay.clear();

        dsDisplay.addElementData( "LEFT_RPM" , robotDrive->getLeftRate() );
        dsDisplay.addElementData( "RIGHT_RPM" , robotDrive->getRightRate() );
        dsDisplay.addElementData( "LEFT_DIST" , robotDrive->getLeftDist() );
        dsDisplay.addElementData( "RIGHT_DIST", robotDrive->getRightDist() );


        dsDisplay.sendToDS();
    }

    dsDisplay.receiveFromDS();

 //   insight->receiveFromDS();
}

START_ROBOT_CLASS( Robot );

