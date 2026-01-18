---
title: PID Control
nav_order: 4
parent: Advanced
---

## PID Control in INDI Drivers

INDI Library provides a PID (Proportional-Integral-Derivative) controller implementation that drivers can use for smooth, accurate tracking and positioning. PID controllers are particularly useful for telescope mounts, focusers, and any device requiring precise closed-loop control.

### What is PID Control?

A PID controller continuously calculates an error value as the difference between a desired setpoint and a measured process variable, then applies a correction based on proportional, integral, and derivative terms:

- **Proportional (P)**: Produces output proportional to current error. Higher values = faster response but may cause overshoot.
- **Integral (I)**: Accounts for past errors by integrating over time. Eliminates steady-state error but can cause windup.
- **Derivative (D)**: Predicts future error based on rate of change. Provides damping to reduce overshoot.

The output is calculated as:
```
output = Kp * error + Ki * ∫error·dt + Kd * d(error)/dt
```

### PID Class API

The INDI PID class is located in `libs/indibase/pid/pid.h` and provides the following interface:

```cpp
#include "pid.h"

// Constructor parameters:
// dt  - sampling/loop interval time (seconds)
// max - maximum output value
// min - minimum output value  
// Kp  - proportional gain
// Kd  - derivative gain
// Ki  - integral gain
PID(double dt, double max, double min, double Kp, double Kd, double Ki);

// Calculate control output
double calculate(double setpoint, double measurement);

// Reset controller state (clears integral, previous values)
void reset();

// Adjust gains dynamically
void setKp(double Kp);
void setKi(double Ki);
void setKd(double Kd);
void getGains(double &Kp, double &Ki, double &Kd) const;

// Set integrator limits (anti-windup)
void setIntegratorLimits(double min, double max);

// Set derivative filter time constant
void setTau(double value);

// Access individual terms for debugging
double proportionalTerm() const;
double integralTerm() const;
double derivativeTerm() const;
```

### Key Features

1. **Anti-Windup Protection**: Prevents integrator accumulation when output saturates
2. **Derivative on Measurement**: Calculates derivative on process variable (not error) to prevent "derivative kick" during setpoint changes
3. **Low-Pass Filtered Derivative**: Reduces noise sensitivity using configurable time constant (tau)
4. **Trapezoidal Integration**: More accurate than simple rectangular integration

### Practical Example: Telescope Mount Tracking

The Skywatcher Alt-Az mount driver uses PID controllers for tracking celestial objects. Here's a simplified example:

#### 1. Initialize PID Controllers

```cpp
#include "pid.h"

class MyMount : public INDI::Telescope
{
private:
    std::unique_ptr<PID> m_AxisAzController;
    std::unique_ptr<PID> m_AxisAltController;
};

// In initProperties() or connection handler:
void MyMount::setupPIDControllers()
{
    // Sampling time: polling period in seconds
    double dt = getPollingPeriod() / 1000.0;  // Convert ms to seconds
    
    // Output limits: ±1000 (example units)
    double max = 1000.0;
    double min = -1000.0;
    
    // Gains (tune these for your application)
    double Kp_az = 0.1;   // Azimuth proportional gain
    double Ki_az = 0.05;  // Azimuth integral gain
    double Kd_az = 0.05;  // Azimuth derivative gain
    
    // Create controller
    m_AxisAzController.reset(new PID(dt, max, min, Kp_az, Kd_az, Ki_az));
    
    // Set integrator limits to prevent windup
    m_AxisAzController->setIntegratorLimits(-1000, 1000);
    
    // Similar for altitude axis
    double Kp_alt = 0.2;
    double Ki_alt = 0.1;
    double Kd_alt = 0.1;
    m_AxisAltController.reset(new PID(dt, max, min, Kp_alt, Kd_alt, Ki_alt));
    m_AxisAltController->setIntegratorLimits(-1000, 1000);
}
```

#### 2. Use PID in Tracking Loop

```cpp
// Called periodically (e.g., every 100ms) during tracking
void MyMount::trackingLoop()
{
    // Get target position (setpoint) in encoder steps
    long targetAzSteps = calculateTargetAzimuth();
    long targetAltSteps = calculateTargetAltitude();
    
    // Get current position (measurement) from encoders
    long currentAzSteps = getCurrentAzimuthEncoder();
    long currentAltSteps = getCurrentAltitudeEncoder();
    
    // Calculate control output (tracking rate)
    double azTrackRate = m_AxisAzController->calculate(targetAzSteps, currentAzSteps);
    double altTrackRate = m_AxisAltController->calculate(targetAltSteps, currentAltSteps);
    
    // Apply tracking rates to motors
    setMotorRate(AXIS_AZ, azTrackRate);
    setMotorRate(AXIS_ALT, altTrackRate);
    
    // Optional: Log PID terms for tuning
    LOGF_DEBUG("AZ - P: %.2f I: %.2f D: %.2f Output: %.2f",
               m_AxisAzController->proportionalTerm(),
               m_AxisAzController->integralTerm(),
               m_AxisAzController->derivativeTerm(),
               azTrackRate);
}
```

#### 3. Reset When Needed

```cpp
// Reset controller state when starting tracking or after large changes
void MyMount::startTracking()
{
    m_AxisAzController->reset();
    m_AxisAltController->reset();
    TrackState = SCOPE_TRACKING;
}

// Also reset after sync or manual slew
void MyMount::Sync(double ra, double dec)
{
    // Update alignment model...
    
    // Reset PID to prevent integral windup from position jump
    m_AxisAzController->reset();
    m_AxisAltController->reset();
}
```

### Tuning Guidelines

PID tuning is an iterative process. Start with these steps:

1. **Start with P-only control** (Ki=0, Kd=0)
   - Increase Kp until system oscillates
   - Reduce Kp to 50-60% of oscillation point
   
2. **Add Integral term** if steady-state error exists
   - Start with Ki = Kp / 10
   - Increase slowly until steady-state error eliminated
   - Watch for integrator windup
   
3. **Add Derivative term** to reduce overshoot
   - Start with Kd = Kp / 10  
   - Increase if overshoot is problematic
   - Don't overdo it - high Kd amplifies noise

4. **Fine-tune using Ziegler-Nichols or trial-and-error**

### Common Patterns

#### Exposing PID Gains as Properties

Allow users to tune PID gains through the client:

```cpp
// In header:
INDI::PropertyNumber AxisPIDNP{3};
enum { Proportional, Integral, Derivative };

// In initProperties():
AxisPIDNP[Proportional].fill("Proportional", "Proportional", "%.2f", 0.1, 100, 1, 0.1);
AxisPIDNP[Integral].fill("Integral", "Integral", "%.2f", 0, 100, 1, 0.05);
AxisPIDNP[Derivative].fill("Derivative", "Derivative", "%.2f", 0, 100, 1, 0.05);
AxisPIDNP.fill(getDeviceName(), "AXIS_PID", "PID Gains", TRACKING_TAB, IP_RW, 60, IPS_IDLE);

// In ISNewNumber():
if (AxisPIDNP.isNameMatch(name))
{
    AxisPIDNP.update(values, names, n);
    AxisPIDNP.setState(IPS_OK);
    AxisPIDNP.apply();
    
    // Update PID controller with new gains
    m_Controller->setKp(AxisPIDNP[Proportional].getValue());
    m_Controller->setKi(AxisPIDNP[Integral].getValue());
    m_Controller->setKd(AxisPIDNP[Derivative].getValue());
    
    saveConfig(AxisPIDNP);
    return true;
}
```

#### Debugging PID Performance

Log individual terms to understand controller behavior:

```cpp
#ifdef DEBUG_PID
LOGF_DEBUG("PID Terms - P: %8.1f I: %8.1f D: %8.1f Output: %8.1f",
           controller->proportionalTerm(),
           controller->integralTerm(),
           controller->derivativeTerm(),
           output);
#endif
```

### Best Practices

1. **Match Sampling Time to Loop Period**: Set `dt` parameter to actual loop execution time
2. **Set Appropriate Output Limits**: Match hardware capabilities to prevent saturation
3. **Use Integrator Limits**: Always set integrator limits to prevent windup
4. **Reset After Large Changes**: Call `reset()` after syncs, park/unpark, or manual slews
5. **Log for Tuning**: Use `proportionalTerm()`, `integralTerm()`, `derivativeTerm()` to understand behavior
6. **Start Conservative**: Begin with low gains and increase gradually
7. **Save Working Gains**: Allow users to save tuned gains to config file

### Common Issues and Solutions

| Problem | Likely Cause | Solution |
|---------|--------------|----------|
| Oscillation around target | Kp too high | Reduce proportional gain |
| Slow to reach target | Kp too low | Increase proportional gain |
| Never quite reaches target | No integral term | Add/increase Ki |
| Overshoots and slow to settle | Kd too low | Increase derivative gain |
| Very noisy output | Kd too high or noisy measurement | Reduce Kd, increase tau, filter measurements |
| Sudden output jump on setpoint change | Derivative on error | PID class already handles this correctly (derivative on measurement) |
| Integrator keeps growing when stuck | No anti-windup | PID class includes anti-windup, ensure output limits are set |

### Advanced Topics

#### Gain Scheduling

Adjust gains based on operating conditions:

```cpp
// Use different gains at different speeds or positions
if (slewSpeed > HIGH_SPEED_THRESHOLD)
{
    controller->setKp(HIGH_SPEED_KP);
    controller->setKd(HIGH_SPEED_KD);
}
else
{
    controller->setKp(NORMAL_KP);
    controller->setKd(NORMAL_KD);
}
```

#### Predictive Tracking

Combine PID with feed-forward for better tracking:

```cpp
// Calculate expected rate (feed-forward)
double predictedRate = calculatePredictedTrackingRate();

// Add PID correction for errors
double pidCorrection = controller->calculate(target, current);

// Combined output
double output = predictedRate + pidCorrection;
```

### References

- [PID Implementation](https://github.com/indilib/indi/tree/master/libs/indibase/pid)
- [Skywatcher Mount Driver Example](https://github.com/indilib/indi/blob/master/drivers/telescope/skywatcherAPIMount.cpp)
- [PID Control Theory](https://en.wikipedia.org/wiki/PID_controller)

### See Also

- [Loops](loops.md) - Understanding driver execution loops
- [Logging](logging.md) - Debugging PID performance
