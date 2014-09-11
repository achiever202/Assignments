#!/bin/bash

# Printing the system information such as system name, ip, date, uptime, os.
echo -e "1. System Information:"

# Extracting the system name from the file hostname.
system_name=`hostname`
echo -e "\tSystem Name: $system_name"

# Getting the ip address of the system.
ip=$(ifconfig | grep "inet addr" | awk '{print $2}' | head -n 1 | cut -d":" -f2)
echo -e "\tIP Address: $ip"

# Geting the date.
today=$(date | cut -d' ' -f2,3,6)
echo -e "\tDate: $today"

# Getting the uptime of the system
up_time=$(uptime | awk '{print $3}' | cut -d',' -f1)
echo -e "\tSystem uptime: $up_time"

#Getting the operating system
os=$(uname -a | cut -d'#' -f1)
echo -e "\tOperating System: $os"

# Printing the hardware and CPU informations using the lscpu command.
# lscpu command lists all the CPU and general hardware configurations.
echo -e "\n2. Hardware and CPU Information:"

# Getting the type of CPU
cpu=$(lscpu | grep "Architecture")
cpu=$(echo -e $cpu | cut -d":" -f2)
echo -e "\tCPU Type: $cpu"

# Getting the number of processeors.
cpu=$(lscpu | grep "CPU(s)")
cpu=$(echo -e $cpu | cut -d":" -f2)
cpu=$(echo -e $cpu | cut -d" " -f1)
echo -e "\tNumber of processors: $cpu"

# Getting the op-modes of CPU.
cpu=$(lscpu | grep "CPU op-mode(s)")
cpu=$(echo -e $cpu | cut -d":" -f2)
echo -e "\tCPU op-mode(s): $cpu"

# Getting the CPU Vendor
cpu=$(lscpu | grep "Vendor")
cpu=$(echo -e $cpu | cut -d":" -f2)
cpu=$(echo -e $cpu | cut -d" " -f1)
echo -e "\tVendor ID: $cpu"

# Getting the CPU Frequency
cpu=$(lscpu | grep "CPU MHz")
cpu=$(echo -e $cpu | cut -d":" -f2)
cpu=$(echo -e $cpu | cut -d" " -f1)
echo -e "\tCPU MHz: $cpu"

# Printing the memory configurations of the system usinf free and df commands.
# free lists the RAM properties of the system.
# df list the various disks on the system.
echo -e "\n3. Memory Information:"

# Getting the size of the main memory.
ram_size=$(free -m | grep 'Mem' | awk '{print $2}')
echo -e "\tMain memory size: $ram_size"

# Getting the total number of disks
total_disks=$(df | wc -l)
total_disks=$((total_disks-1))
echo -e "\tTotal number of disks: $total_disks"

# Printing the information about each disk using the df command listed earlier.
echo -e "\n4. Disks Information:"
echo -e "\tDetails of each disk (in MBs) are:"

# Printing the column headers, separated by tabs.
echo -e "\tDisk\t\tTotal Size\tUsed\t\tFree"

count=-1
df -m | while read -r LINE
do
	count=$((count+1))
	if [ "$count" -gt 0 ]; then
		# getting the total size of the disk.
		total_size=$(echo $LINE|cut -d" " -f2)
		
		# getting the used size of the disk.
		used=$(echo $LINE|cut -d" " -f3)

		# getting the free size of the disk.
		free=$(echo $LINE|cut -d" " -f4)

		echo -e "\tDisk-$count\t\t$total_size\t\t$used\t\t$free"
	fi
done

# Printing the instantaneous RAM usage using the free command, listed earlier.
echo -e "\n5. Instantaneous RAM Usage (in MBs):"

# Getting the used RAM.
ram_used=$(free -m | grep 'Mem' | awk '{print $3}')
echo -e "\tRAM Used: $ram_used"

# Getting the free RAM.
ram_free=$(free -m | grep 'Mem' | awk '{print $4}')
echo -e "\tRAM Free: $ram_free"

# Getting the shared RAM.
ram_shared=$(free -m | grep 'Mem' | awk '{print $5}')
echo -e "\tRAM Shared: $ram_shared"

# Getting the buffers.
buffers=$(free -m | grep 'Mem' | awk '{print $6}')
echo -e "\tBuffer: $buffers"

# Getting the cache.
cache=$(free -m | grep 'Mem' | awk '{print $7}')
echo -e "\tCache: $cache"

# Printing the instantaneous CPU Usgae, using top command.
echo -e "\n6. Instantaneous CPU Usage information:"

# Getting the un-niced user processes time.
cpu=$(top -b -n 1 | grep "Cpu")
us=$(echo -e $cpu | cut -d" " -f2)
echo -e "\tTime running un-niced user processes: $us"

# Getting the system user processes time.
sy=$(echo -e $cpu | cut -d" " -f4)

# Getting the niced user processes time.
ni=$(echo -e $cpu | cut -d" " -f6)
echo -e "\tTime running niced user processes: $ni"
echo -e "\tTime running kernel processes: $sy"

# Getting the IO wait time.
wa=$(echo -e $cpu | cut -d" " -f10)
echo -e "\tTime waiting for IO completion: $wa"

# Getting the interrupt time.
hi=$(echo -e $cpu | cut -d" " -f12)
echo -e "\tTime spent serving hardware interrupts: $hi"

si=$(echo -e $cpu | cut -d" " -f14)
echo -e "\tTime spent serving software interrupts: $si"

# Printing the user Information, using the passwd file and the who command.
# passwd file stores all the users.
# who command states the logged in users.
echo -e "\n7. User Information:"

# Getting the total and current users.
total=$(cat ../../../../../etc/passwd | wc -l)
current=$(who -m | wc -l)
echo -e "\tTotal Users: $total"
echo -e "\tCurrently Logged in Users: $current"

# Printing the jobs Information using ps command.
# ps command lists all the processes running on the system.
echo -e "\n8. Processes Information:"

# Getting the total processes.
total_processes=$(ps -ef | wc -l)
echo -e "\tTotal Processes: $total_processes"

# Getting the user_processes, across all the logged in users.
current_users=$(who -m | awk '{print $1}')
user_processes=0

# summing up processes across each user.
while read LINE;
do
	temp_processes=$(ps -ef | grep "$LINE" | wc -l)
	user_processes=$((user_processes + temp_processes))
done <<<"$current_users"
echo -e "\tUser Processes: $user_processes"

# Getting the kernel processes from the above.
kernel_processes=$((total_processes-user_processes))
echo -e "\tKernel Processes: $kernel_processes"
