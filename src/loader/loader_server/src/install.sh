echo "Installing service..."

# Copy service file to directory
cp "loader.service" "/lib/systemd/system/loader.service"

# Install and start the service
systemctl daemon-reload
systemctl enable loader
systemctl start loader 
echo "Service installed"