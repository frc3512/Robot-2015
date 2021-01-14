// Copyright (c) 2015-2021 FRC Team 3512. All Rights Reserved.

#include "Robot.hpp"

Robot::Robot() {
    autonChooser.AddAutonomous("DriveForward", [=] { AutoDriveForward(); });
    autonChooser.AddAutonomous("ResetElevator", [=] { AutoResetElevator(); });
    autonChooser.AddAutonomous("OneCanLeft", [=] { AutoOneCanLeft(); });
    autonChooser.AddAutonomous("OneCanCenter", [=] { AutoOneCanCenter(); });
    autonChooser.AddAutonomous("OneCanRight", [=] { AutoOneCanRight(); });
    autonChooser.AddAutonomous("OneTote", [=] { AutoOneTote(); });
}

void Robot::TeleopPeriodic() {
    drivetrain.Drive(driveStick1.GetY(), driveStick2.GetX(),
                     driveStick2.GetRawButton(2));

    // Open/close tines
    if (appendageStick.GetRawButtonPressed(1)) {
        elevator.ElevatorGrab(!elevator.IsElevatorGrabbed());
    }

    // Open/close intake
    if (appendageStick.GetRawButtonPressed(2)) {
        elevator.IntakeGrab(!elevator.IsIntakeGrabbed());
    }

    // Start auto-stacking mode
    if (appendageStick.GetRawButtonPressed(3)) {
        elevator.StackTotes();
    }

    // Manual height control
    if (appendageStick.GetRawButtonPressed(4)) {
        elevator.SetManualMode(!elevator.IsManualMode());
    }

    // Stow intake
    if (appendageStick.GetRawButtonPressed(5)) {
        elevator.StowIntake(!elevator.IsIntakeStowed());
    }

    // Open/close container grabber
    if (appendageStick.GetRawButtonPressed(6)) {
        elevator.ContainerGrab(!elevator.IsContainerGrabbed());
    }

    // Automatic preset buttons (7-12)
    if (appendageStick.GetRawButtonPressed(8)) {
        elevator.RaiseElevator(Elevator::kGroundHeight);
    }
    if (appendageStick.GetRawButtonPressed(7)) {
        elevator.RaiseElevator(Elevator::kToteHeight1);
    }
    if (appendageStick.GetRawButtonPressed(10)) {
        elevator.RaiseElevator(Elevator::kToteHeight2);
    }
    if (appendageStick.GetRawButtonPressed(9)) {
        elevator.RaiseElevator(Elevator::kToteHeight3);
    }
    if (appendageStick.GetRawButtonPressed(12)) {
        elevator.RaiseElevator(Elevator::kToteHeight4);
    }
    if (appendageStick.GetRawButtonPressed(11)) {
        elevator.RaiseElevator(Elevator::kToteHeight5);
    }

    // Set manual value
    elevator.SetManualLiftSpeed(appendageStick.GetY() * 12_V);

    // Controls intake
    if (driveStick2.GetPOV() == 0 || appendageStick.GetPOV() == 0) {
        elevator.SetIntakeDirection(Elevator::S_FORWARD);
    } else if (driveStick2.GetPOV() == 90 || appendageStick.GetPOV() == 90) {
        elevator.SetIntakeDirection(Elevator::S_ROTATE_CCW);
    } else if (driveStick2.GetPOV() == 180 || appendageStick.GetPOV() == 180 ||
               driveStick2.GetRawButton(1)) {
        elevator.SetIntakeDirection(Elevator::S_REVERSE);
    } else if (driveStick2.GetPOV() == 270 || appendageStick.GetPOV() == 270) {
        elevator.SetIntakeDirection(Elevator::S_ROTATE_CW);
    } else {
        elevator.SetIntakeDirection(Elevator::S_STOPPED);
    }

    elevator.UpdateState();
}

void Robot::AutonomousInit() { drivetrain.ResetEncoders(); }

void Robot::AutonomousPeriodic() {
    autonChooser.AwaitRunAutonomous();

    elevator.UpdateState();
}

#ifndef RUNNING_FRC_TESTS
int main() { return frc::StartRobot<Robot>(); }
#endif
