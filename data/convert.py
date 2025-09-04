import json
import numpy as np

# CONVERTS input files as used by HYBRID-DIBR to general files used by COLMAP.
# The reverse operation should be obvious from this program

def extract_rotation_euler(matrix):
    # Assuming no shear and scale = 1
    # Extract rotation matrix (upper-left 3x3)
    R = matrix[:3, :3]

    # Extract Euler angles (XYZ order)
    sy = np.sqrt(R[0, 0] ** 2 + R[1, 0] ** 2)

    singular = sy < 1e-6

    if not singular:
        x = np.arctan2(R[2, 1], R[2, 2])
        y = np.arctan2(-R[2, 0], sy)
        z = np.arctan2(R[1, 0], R[0, 0])
    else:
        x = np.arctan2(-R[1, 2], R[1, 1])
        y = np.arctan2(-R[2, 0], sy)
        z = 0

    return np.degrees([x, y, z]).tolist()

def convert_to_colmap(input_file, output_file):
    # Open and read the original JSON file
    with open(input_file, 'r', encoding='utf-8') as infile:
        data = json.load(infile)

    cameras = [{
        "NameColor": "viewport",
        "Position": np.array(data["cameras"][0]["model"])[:3, 3].tolist(),
        "Rotation": extract_rotation_euler(np.array(data["cameras"][0]["model"])),
        "Depth_range": [0.1, 100.0],
        "Resolution": data["Resolution"],
        "Projection": "Perspective",
        "Focal": data["Focal"],
        "Principle_point": data["cameras"][0]["Principal_point"],
    }]

    for camera in data["cameras"]:
        cameras.append({
            "NameColor": camera["NameColor"],
            "NameDepth": camera["NameDepth"],
            "Position": np.array(camera["model"])[:3, 3].tolist(),
            "Rotation": extract_rotation_euler(np.array(camera["model"])),
            "Depth_range": camera["Depth_range"],
            "Resolution": data["Resolution"],
            "Projection": "Perspective",
            "Focal": data["Focal"],
            "Principle_point": camera["Principal_point"],
            "BitDepthColor": 8,
            "BitDepthDepth": 16
        })

    # Wrap the data in an array
    newdata = {"Axial_system": "COLMAP", "cameras": cameras}

    # Write the wrapped data to the new JSON file
    with open(output_file, 'w', encoding='utf-8') as outfile:
        json.dump(newdata, outfile, indent=4, ensure_ascii=False)

def main():
    convert_to_colmap('<folder-to/input-file>.json','<folder-to/output-file>.json')


if __name__ == "__main__":
    main()