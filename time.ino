//=======================================================================
//  updateWake: calculate next time to wake
//=======================================================================
void updateWake (void)
{
  time(&now);
  nextUpdate = UpdateIntervalSeconds - now % UpdateIntervalSeconds;
}
