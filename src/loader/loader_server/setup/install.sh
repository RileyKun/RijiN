# REMEMBER TO RUN dos2unix on this file when editing it from windows!!!

echo "Installing service..."

# Copy service file to directory
cp "loader.service" "/lib/systemd/system/loader.service"

sudo apt-get install libcap2-bin
sudo setcap cap_net_bind_service=+ep `readlink -f \`which node\``

# Install and start the service
systemctl daemon-reload
systemctl enable loader
systemctl start loader
 
echo "Service installed"

# Copy bashrc to root
echo "Setting up bashrc..."

rm -f ~/.bashrc

# only run on putty session
echo 'if [ "$SSH_TTY" ]' >> ~/.bashrc 
echo "then" >> ~/.bashrc 

# aliases
echo "	alias status='systemctl status --no-pager loader'" >> ~/.bashrc 
echo "	alias log='status && journalctl -f -u loader'" >> ~/.bashrc 
echo "	alias restart='systemctl restart loader && status'" >> ~/.bashrc 
echo "	alias start='systemctl start loader && status'" >> ~/.bashrc 
echo "	alias stop='systemctl stop loader && status'" >> ~/.bashrc 
echo "	alias http='tail -f /var/log/nginx/access.log'" >> ~/.bashrc
echo "	alias httpc='> /var/log/nginx/access.log'" >> ~/.bashrc

# cd 
echo "	cd /home/loader" >> ~/.bashrc 

# Show status
echo "	clear" >> ~/.bashrc 
echo "	systemctl status --no-pager loader" >> ~/.bashrc 

# Print commands
echo "	echo status - show loader status" >> ~/.bashrc 
echo "	echo log - show loader log" >> ~/.bashrc 
echo "	echo restart - restart loader" >> ~/.bashrc 
echo "	echo start - start loader" >> ~/.bashrc 
echo "	echo stop - stop loader" >> ~/.bashrc 
echo "	echo http - view http traffic" >> ~/.bashrc 
echo "	echo httpc - clear http traffic logs" >> ~/.bashrc 

# end
echo "fi" >> ~/.bashrc 

#
echo "Fully installed"