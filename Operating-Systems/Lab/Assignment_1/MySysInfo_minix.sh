#!/bin/bash

# Printing the system information such as system name, ip, date, uptime, os.
echo -e "1. System Information:"

# Extracting the system name from the file hostname.
system_name=$(cat "/etc/hostname.file") 
echo -e "\tSystem Name: $system_name"

# Getting the ip address of the system.
ip=$(ifconfig | awk '{print $3}')
echo -e "\tIP Address: $ip"

# Geting the date.
today=$(date | cut -d' ' -f2,3,6)
echo -e "\tDate: $today"

# Getting the uptime of the system
up_time=$(uptime | awk '{print $3}')
echo -e "\tSystem uptime: $up_time"

#Getting the operating system
os=$(uname)
echo -e "\tOperating System: $os"

... check cpuinfo and meminnfo

# Printing the hardware and CPU informations using the lscpu command.
# lscpu command lists all the CPU and general hardware configurations.
echo -e "\n2. Hardware and CPU Information:"

# Getting the number of processeors.
cpu=$(cat /proc/cpuinfo | grep "processor" | wc -l)
echo -e "\tNumber of processors: $cpu"

# Getting the CPU Vendor
cpu=$(cat /proc/cpuinfo | grep "vendor_id" | awk '{print $3}')
echo -e "\tVendor ID: $cpu"

# Getting the model name
cpu=$(cat /proc/cpuinfo | grep "model name" | awk '{print $4}')
echo -e "\tVendor ID: $cpu"

# Getting the CPU Frequency
cpu=$(cat /proc/cpuinfo | grep "cpu MHz" | awk '{print $4}')
echo -e "\tVendor ID: $cpu"

# Printing the memory configurations of the system usinf df command.
# df list the various disks on the system.
echo -e "\n3. Memory Information:"

# Getting the total number of disks
total_disks=$(df | tail -n +2 | wc -l)
echo -e "\tTotal number of disks: $total_disks"

# Printing the information about each disk using the df command listed earlier.
echo -e "\n4. Disks Information:"
echo -e "\tDetails of each disk (in MBs) are:"

# Printing the column headers, separated by tabs.
df | awk '{print $1 "\t" $2 "\t" $3 "\t" $4}'

# Printing the user Information, using the passwd file and the who command.
# passwd file stores all the users.
# who command states the logged in users.
echo -e "\n7. User Information:"

# Getting the total users.
total=$(cat /etc/passwd | wc -l)
echo -e "\tTotal Users: $total"

current_users=$(who | wc -l)
echo -e "\tCurrent Users: $current_users"

# Printing the jobs Information using ps command.
# ps command lists all the processes running on the system.
echo -e "\n8. Processes Information:"

# Getting the total processes.
total_processes=$(ps -ef | wc -l)
echo -e "\tTotal Processes: $total_processes"
