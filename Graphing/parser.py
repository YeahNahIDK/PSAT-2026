import csv
import math

# File names - change these if your files are named differently
INPUT_FILE = "flight_data.csv"
OUTPUT_FILE = "processed_flight_data.csv"

def process_flight_data():
    with open(INPUT_FILE, mode='r') as infile, open(OUTPUT_FILE, mode='w', newline='') as outfile:
        reader = csv.DictReader(infile)
        
        # Define and write the exact header you requested
        fieldnames = [
            "Time", "Disp_X", "Disp_Y", "Disp_Z", "Vel_X", "Vel_Y", "Vel_Z", 
            "Acc_X", "Acc_Y", "Acc_Z", "Ang_R", "Ang_P", "Ang_Y", "Pressure", "Temp"
        ]
        writer = csv.DictWriter(outfile, fieldnames=fieldnames)
        writer.writeheader()

        # State tracking variables for initial offsets and integration
        first_row = True
        start_time_ms = 0
        start_lat = 0.0
        start_lon = 0.0
        start_alt = 0.0
        
        prev_time_s = 0.0
        prev_disp_x = 0.0
        prev_disp_y = 0.0
        prev_disp_z = 0.0
        
        ang_r = 0.0
        ang_p = 0.0
        ang_y = 0.0

        for row in reader:
            try:
                # Read raw values
                time_ms = float(row["Time_ms"])
                lat = float(row["Latitude"])
                lon = float(row["Longitude"])
                alt = float(row["Altitude_m"])
                temp = float(row["Temp_C"])
                pressure = float(row["Pressure_Pa"])
                acc_x = float(row["Acc_X"])
                acc_y = float(row["Acc_Y"])
                acc_z = float(row["Acc_Z"])
                gyr_x = float(row["Gyro_X"])
                gyr_y = float(row["Gyro_Y"])
                gyr_z = float(row["Gyro_Z"])

                # Capture baseline data on the very first loop
                if first_row:
                    start_time_ms = time_ms
                    start_lat = lat
                    start_lon = lon
                    start_alt = alt
                    first_row = False
                    
                    # First row output: everything starts at zero
                    writer.writerow({
                        "Time": 0, "Disp_X": "0.0", "Disp_Y": "0.0", "Disp_Z": "0.0",
                        "Vel_X": "0.0", "Vel_Y": "0.0", "Vel_Z": "0.0",
                        "Acc_X": f"{acc_x:.1f}", "Acc_Y": f"{acc_y:.1f}", "Acc_Z": f"{acc_z:.1f}",
                        "Ang_R": "0.0", "Ang_P": "0.0", "Ang_Y": "0.0",
                        "Pressure": f"{int(pressure)}", "Temp": f"{temp:.1f}"
                    })
                    continue

                # --- 1. Time Calculation ---
                current_time_s = (time_ms - start_time_ms) / 1000.0
                dt = current_time_s - prev_time_s
                
                if dt <= 0:
                    continue # Skip duplicate time entries to avoid divide-by-zero errors

                # --- 2. Displacement Calculations (Flat Earth Approximation) ---
                delta_lat = lat - start_lat
                delta_lon = lon - start_lon
                
                # 1 degree lat is approx 111,320 meters
                disp_y = delta_lat * 111320.0 
                # Longitude distance depends on current latitude
                disp_x = delta_lon * 111320.0 * math.cos(math.radians(start_lat))
                
                disp_z = alt - start_alt

                # --- 3. Velocity Calculations (Derivative of Displacement) ---
                vel_x = (disp_x - prev_disp_x) / dt
                vel_y = (disp_y - prev_disp_y) / dt
                vel_z = (disp_z - prev_disp_z) / dt

                # --- 4. Absolute Angles (Integration of Gyro Rates) ---
                # new_angle = old_angle + (rate_of_change * time_elapsed)
                ang_r += gyr_x * dt
                ang_p += gyr_y * dt
                ang_y += gyr_z * dt

                # --- 5. Write to CSV ---
                writer.writerow({
                    "Time": f"{int(current_time_s)}",
                    "Disp_X": f"{disp_x:.1f}",
                    "Disp_Y": f"{disp_y:.1f}",
                    "Disp_Z": f"{disp_z:.1f}",
                    "Vel_X": f"{vel_x:.1f}",
                    "Vel_Y": f"{vel_y:.1f}",
                    "Vel_Z": f"{vel_z:.1f}",
                    "Acc_X": f"{acc_x:.1f}",
                    "Acc_Y": f"{acc_y:.1f}",
                    "Acc_Z": f"{acc_z:.1f}",
                    "Ang_R": f"{ang_r:.1f}",
                    "Ang_P": f"{ang_p:.1f}",
                    "Ang_Y": f"{ang_y:.1f}",
                    "Pressure": f"{int(pressure)}",
                    "Temp": f"{temp:.1f}"
                })

                # Update states for the next loop
                prev_time_s = current_time_s
                prev_disp_x = disp_x
                prev_disp_y = disp_y
                prev_disp_z = disp_z

            except ValueError:
                # Ignores rows with corrupt/incomplete data (common in SD card writing)
                pass
            except KeyError as e:
                print(f"Error: Missing column {e} in raw data.")
                break

    print(f"Success! Data processed and saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    process_flight_data()
