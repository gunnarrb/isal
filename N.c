float N(float muy, float sigma, int stream)
{
		// This method of converting from N(0,1) to N(muy,sigma) has not been verified!
		float x = nrand(stream);
			return (x*sigma)+30;
}
