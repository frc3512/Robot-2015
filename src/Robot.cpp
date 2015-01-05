#include "Robot.hpp"
#include <cmath>

Robot::Robot() : settings( "RobotSettings.txt" ) ,
                 drive1Buttons( 1 ) ,
                 drive2Buttons( 2 ) ,
                 shootButtons( 3 ) ,
                 pidGraph( 3513 ) {
    robotDrive = new DriveTrain();
    claw = new Claw( 7 , 8 , 2 , 9 );

    driveStick1 = new Joystick( 1 );
    driveStick2 = new Joystick( 2 );
    shootStick = new Joystick( 3 );

    autonTimer = new Timer();
    displayTimer = new Timer();

    driverStation =
        DriverStationDisplay<Robot>::getInstance( settings.getInt( "DS_Port" ) );

    driverStation->addAutonMethod( "DriveForward Autonomous" ,
                                   &Robot::DriveForwardAuton ,
                                   this );
    driverStation->addAutonMethod( "Right/Left Autonomous" ,
                                   &Robot::RightLeftAuton ,
                                   this );
    driverStation->addAutonMethod( "MotionProfile" ,
                                   &Robot::AutonMotionProfile ,
                                   this );
    driverStation->addAutonMethod( "Side Auton" , &Robot::SideAuton , this );
    driverStation->addAutonMethod( "Noop Auton" , &Robot::NoopAuton , this );

    pidGraph.resetTime();
    pidGraph.setSendInterval( 200 );

    insight = Insight::getInstance( settings.getInt( "Insight_Port" ) );

    logger1 = new Logger();
    ls = new LogStream( logger1 );
    logFileSink = new LogFileSink( "LogFile.txt" );
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
    delete claw;

    delete driveStick1;
    delete driveStick2;
    delete shootStick;

    delete autonTimer;
    delete displayTimer;

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

    while ( IsOperatorControl() && IsEnabled() ) {
        // DS_PrintOut();

        // arcade Drive
        if ( driveStick2->GetRawButton( 2 ) ) {
            robotDrive->drive( driveStick1->GetY() , driveStick2->GetZ() ,
                               true );
        }
        else {
            robotDrive->drive( driveStick1->GetY() , driveStick2->GetZ() );
        }

        if ( drive1Buttons.releasedButton( 1 ) ) {
            robotDrive->setGear( !robotDrive->getGear() );
        }

        // Shoots Ball
        if ( ( shootStick->GetRawButton( 1 ) ||
               driveStick2->GetRawButton( 1 ) ) && !claw->IsShooting() ) {
            claw->Shoot();
        }

        // Engage collector
        if ( shootButtons.releasedButton( 2 ) ) {
            claw->SetCollectorMode( !claw->GetCollectorMode() );
        }

        if ( shootStick->GetRawButton( 3 ) ) {
            claw->SetWheelManual( -1 );
        }
        else if ( shootStick->GetRawButton( 4 ) ) {
            claw->SetWheelManual( 1 );
        }
        else {
            claw->SetWheelManual( 0 );
        }

        if ( shootButtons.pressedButton( 7 ) ) {
            claw->SetAngle( 190.0 );
        }
        else if ( shootButtons.pressedButton( 9 ) ) {
            claw->SetAngle( 106.0 );
        }
        else if ( shootButtons.pressedButton( 8 ) ) {
            claw->SetAngle( 57.0 );
        }
        else if ( shootButtons.pressedButton( 11 ) ) {
            // Collector should always be retracted when resetting encoder
            claw->SetCollectorMode( false );
            claw->SetAngle( 0 );
        }
        else if ( shootButtons.pressedButton( 10 ) ) {
            claw->SetAngle( claw->GetTargetAngle() + 3.0f );
        }
        else if ( shootButtons.pressedButton( 12 ) ) {
            claw->SetAngle( claw->GetTargetAngle() - 3.0f );
        }
        if ( drive1Buttons.pressedButton( 2 ) ) {
            robotDrive->setDefencive( !robotDrive->getDefencive() );
        }

        claw->Update();

        if ( drive2Buttons.releasedButton( 8 ) ) {
            robotDrive->reloadPID();
            claw->ReloadPID();
        }
        drive1Buttons.updateButtons();
        drive2Buttons.updateButtons();
        shootButtons.updateButtons();

        DS_PrintOut();

        Wait( 0.01 );
    }
}

void Robot::Autonomous() {
    autonTimer->Reset();
    autonTimer->Start();

    driverStation->execAutonomous();

    autonTimer->Stop();
}

void Robot::Disabled() {
    claw->SetAngle( 0 );
    claw->ManualSetAngle( 0 );
    claw->SetWheelManual( 0 );

    while ( IsDisabled() ) {
        claw->Update();

        DS_PrintOut();

        logServerSink->acceptor( false );
        Wait( 0.1 );
    }
}

void Robot::Test() {
    claw->testClaw();

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
        pidGraph.graphData( claw->GetTargetAngle() , "Target Angle" );
        pidGraph.graphData( claw->GetAngle() , "Angle" );

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

        driverStation->clear();

        DS::AddElementData( driverStation ,
                            "LEFT_RPM" ,
                            robotDrive->getLeftRate() );
        DS::AddElementData( driverStation ,
                            "RIGHT_RPM" ,
                            robotDrive->getRightRate() );
        DS::AddElementData( driverStation ,
                            "LEFT_DIST" ,
                            robotDrive->getLeftDist() );
        DS::AddElementData( driverStation ,
                            "RIGHT_DIST" ,
                            robotDrive->getRightDist() );

        DS::AddElementData( driverStation ,
                            "ANGLE_SET_DISP" ,
                            claw->GetTargetAngle() );
        DS::AddElementData( driverStation , "ANGLE_SET" ,
                            static_cast<int8_t>( claw->GetTargetAngle() /
                                                 175.f * 100.f ) );

        DS::AddElementData( driverStation , "ANGLE_REAL_DISP" ,
                            claw->GetAngle() );
        DS::AddElementData( driverStation , "ANGLE_REAL" ,
                            static_cast<int8_t>( claw->GetAngle() / 175.f *
                                                 100.f ) );

        DS::StatusLight atAngle = DS::StatusLight::inactive;

        if ( claw->GetTargetAngle() > 150.f ) {
            if ( claw->AtAngle() ) {
                atAngle = DS::StatusLight::active;
            }
            else {
                atAngle = DS::StatusLight::standby;
            }
        }
        DS::AddElementData( driverStation , "ANGLE_PICKUP" , atAngle );

        if ( claw->GetTargetAngle() < 150.f && claw->GetTargetAngle() > 50.f ) {
            if ( claw->AtAngle() ) {
                atAngle = DS::StatusLight::active;
            }
            else {
                atAngle = DS::StatusLight::standby;
            }
        }
        else {
            atAngle = DS::StatusLight::inactive;
        }
        DS::AddElementData( driverStation , "ANGLE_SHOOT" , atAngle );

        if ( claw->GetTargetAngle() <= 1.f ) {
            if ( claw->AtAngle() ) {
                atAngle = DS::StatusLight::active;
            }
            else {
                atAngle = DS::StatusLight::standby;
            }
        }
        else {
            atAngle = DS::StatusLight::inactive;
        }
        DS::AddElementData( driverStation , "ANGLE_ZERO" , atAngle );

        driverStation->sendToDS();
    }

    driverStation->receiveFromDS();

    insight->receiveFromDS();
}

START_ROBOT_CLASS( Robot );

