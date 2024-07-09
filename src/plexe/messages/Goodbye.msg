import ManeuverMessage;
// Final message sent by the leader to break the platoon.
packet Goodbye extends ManeuverMessage {
    string msgContent = "Break the platoon";
}