import ManeuverMessage;
// Message sent by a follower to notify the leader that this vehicle reached the right distance.
packet ReadyForGoodbye extends ManeuverMessage {
    string msgContent = "I reached the right distance";
}