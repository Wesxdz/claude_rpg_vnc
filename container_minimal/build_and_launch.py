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

def launch_container(image_name="turbovnc-desktop", tag="latest", container_name="turbovnc-container"):
    """Launch the TurboVNC container"""
    print(f"\n=== Launching container: {container_name} ===")

    # Stop and remove existing container if it exists
    print("Cleaning up any existing container...")
    subprocess.run(["docker", "stop", container_name], check=False, capture_output=True)
    subprocess.run(["docker", "rm", container_name], check=False, capture_output=True)

    # Run the container
    cmd = [
        "docker", "run",
        "-d",  # Run in detached mode
        "--name", container_name,
        "-p", "5901:5901",  # VNC port
        "-p", "5801:5800",  # noVNC/HTTP port (optional)
        "--shm-size=2g",  # Increase shared memory for better performance
        f"{image_name}:{tag}",
        "bash", "-c",
        "su - vncuser -c '/opt/TurboVNC/bin/vncserver :1 -geometry 1920x1080 -depth 24' && tail -f /dev/null"
    ]
    run_command(cmd)

    print(f"✓ Container started successfully: {container_name}")
    print(f"\n=== Connection Information ===")
    print(f"VNC Server: localhost:5901")
    print(f"VNC Password: password")
    print(f"\nTo connect:")
    print(f"  - Use a VNC client (like TigerVNC, RealVNC, or TurboVNC Viewer)")
    print(f"  - Connect to: localhost:5901")
    print(f"  - Enter password: password")
    print(f"\nTo view logs:")
    print(f"  docker logs -f {container_name}")
    print(f"\nTo stop container:")
    print(f"  docker stop {container_name}")

def main():
    IMAGE_NAME = "turbovnc-desktop"
    TAG = "latest"
    CONTAINER_NAME = "turbovnc-container"

    print("=== TurboVNC Desktop Builder & Launcher ===\n")

    try:
        # Build the image
        build_image(
            dockerfile_path=".",
            image_name=IMAGE_NAME,
            tag=TAG
        )

        # Launch the container
        launch_container(
            image_name=IMAGE_NAME,
            tag=TAG,
            container_name=CONTAINER_NAME
        )

        print("\n✓ Setup complete! Container is running.")

    except subprocess.CalledProcessError as e:
        print(f"\n✗ Error occurred: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
