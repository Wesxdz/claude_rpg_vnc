FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install Kubuntu desktop
RUN apt-get update && apt-get install -y \
    wget \
    kubuntu-desktop \
    && rm -rf /var/lib/apt/lists/*

# Install TurboVNC
RUN wget https://sourceforge.net/projects/turbovnc/files/3.1.1/turbovnc_3.1.1_amd64.deb/download -O turbovnc.deb \
    && dpkg -i turbovnc.deb || apt-get install -f -y \
    && rm turbovnc.deb \
    && ls -la /opt/TurboVNC/bin/ || find /usr -name vncpasswd 2>/dev/null

# Install VirtualGL (optional but recommended)
RUN wget https://sourceforge.net/projects/virtualgl/files/3.1/virtualgl_3.1_amd64.deb/download -O virtualgl.deb \
    && dpkg -i virtualgl.deb || apt-get install -f -y \
    && rm virtualgl.deb

# Create VNC user
RUN useradd -m -s /bin/bash vncuser

# Switch to vncuser and setup VNC as root first, then change ownership
RUN mkdir -p /home/vncuser/.vnc && \
    echo "password" -f > /home/vncuser/.vnc/passwd && \
    chmod 600 /home/vncuser/.vnc/passwd && \
    chown -R vncuser:vncuser /home/vncuser/.vnc

# Configure KDE as the desktop environment
RUN echo "#!/bin/bash\nexport DESKTOP_SESSION=plasma\nexec startplasma-x11" > /home/vncuser/.vnc/xstartup.turbovnc && \
    chmod +x /home/vncuser/.vnc/xstartup.turbovnc && \

# https://turbovnc.org/Documentation/Compatibility
# https://turbovnc.org/Documentation/Compatibility#threedwm
# /opt/TurboVNC/bin/vncserver -3dwm
