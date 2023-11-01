import serial
import matplotlib.pyplot as plt

# Open serial port
SERIAL_PORT = 'COM16'
ser         = serial.Serial(SERIAL_PORT, 115200, timeout=1)

# Array for storing ecg value
SAMPLING_DURATION   = 5
SAMPLING_PERIOD     = 0.01
REF_3V3_FACTOR      = 5084004.85
REF_5V_FACTOR       = 3355443.2
SCALE_FACTOR        = 4.6
lead1_val   = []
lead2_val   = []
lead3_val   = []
time_stamp  = []
time        = 0.0
while(time < SAMPLING_DURATION):
    try:
        leads_val = ser.readline().decode()[:-2].split(',')
        lead1_val.append((float(leads_val[0])/REF_5V_FACTOR)/SCALE_FACTOR)
        lead2_val.append((float(leads_val[1])/REF_5V_FACTOR)/SCALE_FACTOR)
        lead3_val.append((float(leads_val[2])/REF_5V_FACTOR)/SCALE_FACTOR)
        time_stamp.append(time)
        time += SAMPLING_PERIOD
    except:
        pass

# Plot
plt.title("3-Lead ECG Plot")
plt.plot(time_stamp, lead1_val, label='Lead 1')
plt.plot(time_stamp, lead2_val, label='Lead 2')
plt.plot(time_stamp, lead3_val, label='Lead 3')
plt.legend()
plt.show()