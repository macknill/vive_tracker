/*	Floating point PID control loop for Microcontrollers
	Copyright (C) 2015 Jesus Ruben Santa Anna Zamudio.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Author website: http://www.geekfactory.mx
	Author e-mail: ruben at geekfactory dot mx
 */
#include "PID.h"

pid_t pid_create(pid_t pid, float kp, float ki, float kd)
{
	pid->compute = false;

	pid_limits(pid, 0, 255);

	// Set default sample time to 100 ms
	//pid->sampletime = 100 * (TICK_SECOND / 1000);
	pid_tune(pid, kp, ki, kd);
	return pid;
}


bool pid_compute(pid_t pid)
{
	// Check if control is enabled
	if (!pid->compute)
		return false;
	
	float in = (pid->input);
	// Compute error
	float error = ((*pid->setpoint)) - in;
        pid->error = error;
	// Compute integral
	pid->iterm += (pid->Ki * error);
	if (pid->iterm > pid->omax)
		pid->iterm = pid->omax;
	else if (pid->iterm < pid->omin)
		pid->iterm = pid->omin;
	// Compute differential on input
	float dinput = in - pid->lastin;
	// Compute PID output
	float out = pid->Kp * error + pid->iterm - pid->Kd * dinput;
	// Apply limit to output value
	if (out > pid->omax)
        {
		out = pid->omax;
                pid->iterm -= (pid->Ki * error);
        }
	else if (out < pid->omin)
        {
		out = pid->omin;
                pid->iterm -= (pid->Ki * error);
        }
	// Output to pointed variable
	(pid->output) = out;
	// Keep track of some variables for next execution
	pid->lastin = in;
        return true;
}

void pid_tune(pid_t pid, float kp, float ki, float kd)
{
	// Check for validity
	if (kp < 0 || ki < 0 || kd < 0)
		return;

	pid->Kp = kp;
	pid->Ki = ki;
	pid->Kd = kd;
}

void pid_limits(pid_t pid, float min, float max)
{
	if (min >= max) return;
	pid->omin = min;
	pid->omax = max;
	//Adjust output to new limits
	if (pid->compute) {
		if ((pid->output) > pid->omax)
			(pid->output) = pid->omax;
		else if ((pid->output) < pid->omin)
			(pid->output) = pid->omin;

		if (pid->iterm > pid->omax)
			pid->iterm = pid->omax;
		else if (pid->iterm < pid->omin)
			pid->iterm = pid->omin;
	}
}

void pid_auto(pid_t pid)
{
	// If going from manual to auto
	if (!pid->compute) {
		pid->iterm = (pid->output);
		pid->lastin = (pid->input);
		if (pid->iterm > pid->omax)
			pid->iterm = pid->omax;
		else if (pid->iterm < pid->omin)
			pid->iterm = pid->omin;
		pid->compute = true;
	}
}

