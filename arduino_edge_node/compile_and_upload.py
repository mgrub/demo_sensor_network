import os
import subprocess
import shlex
import json

# paths windows
arduino_cli_path = "C:\\Users\\gruber04\\Downloads\\arduino-cli_0.21.1_Windows_64bit\\arduino-cli.exe"
work_dir = ".\\arduino_edge_node\\"
posix = False

# paths linux
# arduino_cli_path = os.path.expanduser("~/programs/arduino-cli_0.22.0_Linux_64bit/arduino-cli")
# work_dir = "./arduino_edge_node/"
# posix = True

# commands
compilation = "{EXE} compile -b arduino:samd:nano_33_iot {WORK_DIR}"
upload = "{EXE} upload -b arduino:samd:nano_33_iot -p {PORT} {WORK_DIR}"


# template for sensors.h
with open("arduino_edge_node/sensor_template.h", "r") as f:
    sensors_header_file_template = f.read()

# get sensors to
sensors_to_work_with = json.load(open("arduino_edge_node/sensors.json", "r"))

for sensor_id, settings in sensors_to_work_with.items():

    # load sensor description
    if os.path.exists(settings["description_file"]):
        with open(settings["description_file"], "r") as f:
            sensor_description = f.read()
            sensor_description = sensor_description.replace("\n", " \\n")
    else:
        continue

    # adjust template
    sensor_header = sensors_header_file_template.format(
        SENSOR_ID=sensor_id, SENSOR_DESCRIPTION=sensor_description,
    )

    # create sensors.h
    with open("arduino_edge_node/sensor.h", "w") as f:
        sensor_description = f.write(sensor_header)
        f.close()

    # compile
    cmd = shlex.split(compilation.format(EXE=arduino_cli_path, WORK_DIR=work_dir), posix=posix)
    print(" ".join(cmd))
    result = subprocess.run(cmd, capture_output=True)
    print(result.stderr)
    print(result.stdout)

    # (try to) upload
    cmd = shlex.split(upload.format(EXE=arduino_cli_path, WORK_DIR=work_dir, PORT=settings["port"]), posix=posix)
    print(" ".join(cmd))
    result = subprocess.run(cmd, capture_output=True)
    print(result.stderr)
    print(result.stdout)

    # clean up
    os.remove("arduino_edge_node/sensor.h")
