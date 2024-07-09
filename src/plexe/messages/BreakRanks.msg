import ManeuverMessage;
// Message sent by a follower to notify the leader that this vehicle reached the right distance.
packet BreakRanks extends ManeuverMessage {
    double safetyDistance; 
}