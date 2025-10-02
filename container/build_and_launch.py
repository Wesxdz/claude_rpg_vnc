#!/usr/bin/env python3
import subprocess
import sys
import time

def run_command(cmd, check=True):
    """Run a shell command and print output"""
    print(f"Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, check=check, capture_output=False, text=True)
    return result

def build_image(dockerfile_path=".", image_name="turbovnc-desktop", tag="latest"):
    """Build Docker image"""
    print(f"\n=== Building Docker image: {image_name}:{tag} ===")
    cmd = [
        "docker", "build",
        "-t", f"{image_name}:{tag}",
        dockerfile_path
    ]
    run_command(cmd)
    print(f"✓ Image built successfully: {image_name}:{tag}")

def main():
    IMAGE_NAME = "turbovnc-desktop"
    TAG = "latest"

    print("=== TurboVNC Desktop Builder & Launcher ===\n")

    try:
        build_image(
            dockerfile_path=".",
            image_name=IMAGE_NAME,
            tag=TAG
        )

    except subprocess.CalledProcessError as e:
        print(f"\n✗ Error occurred: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
