#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Script Function:
1. Assume script is running inside DVP2-Linux64 directory
2. Check if install.sh exists in current directory
3. Check and unload loaded dt_pcie and dsfilter kernel modules
4. Execute install.sh (requires sudo)
5. For modules that are not successful, enter corresponding subdirectories to run sudo make:
   - dt_pcie -> dtpcie/ (check dt_pcie.ko)
   - dsfilter -> filter/ (check dsfilter.ko)
6. Check module status (lsmod and .ko file), record success source and detection method
7. Generate detailed logs (success/failure stored separately)
"""

import os
import subprocess
import sys
import time
from datetime import datetime

# Global output collection
terminal_output = []

def log_print(msg):
    """Print to terminal and save to global list"""
    print(msg)
    terminal_output.append(msg + "\n")

def run_command(cmd, capture=True, need_sudo=False, cwd=None):
    """Execute command, return (returncode, stdout, stderr)"""
    if need_sudo and not (isinstance(cmd, list) and cmd[0] == "sudo"):
        cmd = ["sudo"] + cmd if isinstance(cmd, list) else f"sudo {cmd}"
    try:
        if isinstance(cmd, str):
            result = subprocess.run(cmd, shell=True, capture_output=capture, text=True, cwd=cwd)
        else:
            result = subprocess.run(cmd, capture_output=capture, text=True, cwd=cwd)
        return result.returncode, result.stdout, result.stderr
    except Exception as e:
        return -1, "", str(e)

def is_module_loaded(module_name):
    """Check if module is loaded in kernel (lsmod)"""
    ret, stdout, _ = run_command(f"lsmod | grep -w {module_name}", need_sudo=False)
    return ret == 0 and module_name in stdout

def check_module_ko_file(module_name):
    """
    Check if module .ko file exists
    dt_pcie -> dtpcie/dt_pcie.ko
    dsfilter -> filter/dsfilter.ko
    """
    if module_name == "dt_pcie":
        ko_path = os.path.join("dtpcie", f"{module_name}.ko")
    elif module_name == "dsfilter":
        ko_path = os.path.join("filter", f"{module_name}.ko")
    else:
        return False
    
    full_path = os.path.abspath(ko_path)
    exists = os.path.isfile(full_path)
    if exists:
        log_print(f"  Detected file: {full_path}")
    return exists

def check_module_status(module_name):
    """
    Check module status, returns (is_success, detection_method)
    detection_method: "lsmod", "ko_file", "both", "none"
    """
    lsmod_loaded = is_module_loaded(module_name)
    ko_exists = check_module_ko_file(module_name)
    
    if lsmod_loaded and ko_exists:
        return True, "both"
    elif lsmod_loaded:
        return True, "lsmod"
    elif ko_exists:
        return True, "ko_file"
    else:
        return False, "none"

def unload_module(module_name):
    """Unload kernel module"""
    log_print(f"Unloading module: {module_name}")
    ret, _, stderr = run_command(f"sudo rmmod {module_name}", need_sudo=False)
    if ret == 0:
        log_print(f"✓ Successfully unloaded module: {module_name}")
        return True
    elif "not loaded" in stderr or "not found" in stderr:
        log_print(f"Module {module_name} is not loaded, no need to unload")
        return True
    else:
        log_print(f"✗ Failed to unload module {module_name}: {stderr}")
        return False

def get_system_info():
    """Get system information: uname -r, uname -a, lsb_release -a"""
    info = {}
    ret, stdout, _ = run_command("uname -r", need_sudo=False)
    info["uname -r"] = stdout.strip() if ret == 0 else "Failed to get"
    ret, stdout, _ = run_command("uname -a", need_sudo=False)
    info["uname -a"] = stdout.strip() if ret == 0 else "Failed to get"
    ret, stdout, _ = run_command("lsb_release -a", need_sudo=False)
    info["lsb_release -a"] = stdout.strip() if ret == 0 else "Command not found or execution failed"
    return info

def try_install_module_via_make(module_name):
    """
    Enter corresponding subdirectory to run sudo make
    dt_pcie -> ./dtpcie/
    dsfilter -> ./filter/
    Returns (success, error_log, detection_method)
    """
    if module_name == "dt_pcie":
        sub_path = "dtpcie"
    elif module_name == "dsfilter":
        sub_path = "filter"
    else:
        return False, f"Unknown module name: {module_name}", "none"
    
    module_dir = os.path.abspath(sub_path)
    if not os.path.isdir(module_dir):
        return False, f"Directory does not exist: {module_dir}", "none"
    
    log_print(f"\n--- Attempting to install {module_name} module via sudo make ---")
    log_print(f"Entering directory: {module_dir}")
    
    ret, stdout, stderr = run_command("sudo make", need_sudo=False, cwd=module_dir)
    output = stdout + stderr
    if ret == 0:
        log_print(f"sudo make executed successfully, checking module status...")
        time.sleep(1)
        success, detection = check_module_status(module_name)
        if success:
            log_print(f"✓ Module {module_name} succeeded (detection method: {detection})")
            return True, "", detection
        else:
            error_msg = f"sudo make succeeded but module {module_name} is not ready (neither lsmod nor .ko file detected)"
            log_print(f"✗ {error_msg}")
            return False, error_msg + "\n" + output, "none"
    else:
        error_msg = f"sudo make failed with return code: {ret}"
        log_print(f"✗ {error_msg}")
        return False, error_msg + "\n" + output, "none"

def save_log(overall_success, modules_status, modules_source, modules_detection, make_errors):
    """
    Save log to install_log/success or install_log/error directory
    modules_status: dict {module_name: bool} whether successful
    modules_source: dict {module_name: str} success source, e.g., "install.sh" or "sudo make"
    modules_detection: dict {module_name: str} detection method, e.g., "lsmod", "ko_file", "both"
    make_errors: dict {module_name: str} sudo make error output (only when failed)
    """
    log_base_dir = "install_log"
    os.makedirs(log_base_dir, exist_ok=True)
    sub_dir = "success" if overall_success else "error"
    log_dir = os.path.join(log_base_dir, sub_dir)
    os.makedirs(log_dir, exist_ok=True)
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = os.path.join(log_dir, f"{timestamp}.log")
    sys_info = get_system_info()
    
    with open(log_file, 'w', encoding='utf-8') as f:
        f.write("=" * 60 + "\n")
        f.write("DVP2-Linux64 Module Installation Report\n")
        f.write(f"Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write("=" * 60 + "\n\n")
        
        # Module loading results (at the beginning)
        f.write("【Module Loading Results】\n")
        for module in ['dt_pcie', 'dsfilter']:
            status = "SUCCESS" if modules_status.get(module, False) else "FAILED"
            source = modules_source.get(module, "")
            detection = modules_detection.get(module, "")
            if status == "SUCCESS":
                f.write(f"  {module}: {status} (Source: {source}, Detection: {detection})\n")
            else:
                f.write(f"  {module}: {status}\n")
        f.write("\n")
        
        # Detection method explanation
        f.write("【Detection Method Explanation】\n")
        f.write("  lsmod   : Module is loaded in kernel (detectable via lsmod command)\n")
        f.write("  ko_file : Module .ko file is generated (may require reboot to load)\n")
        f.write("  both    : Both methods detected successfully\n")
        f.write("  none    : Not detected\n\n")
        
        # System information
        f.write("【System Information】\n")
        f.write(f"uname -r : {sys_info['uname -r']}\n")
        f.write(f"uname -a : {sys_info['uname -a']}\n")
        f.write("lsb_release -a :\n")
        f.write(sys_info['lsb_release -a'] + "\n\n")
        
        # sudo make error information
        if make_errors:
            f.write("【sudo make Error Information】\n")
            for module, error in make_errors.items():
                f.write(f"sudo make error for module {module}:\n{error}\n\n")
        
        # Complete terminal output
        f.write("【Script Terminal Output】\n")
        f.writelines(terminal_output)
    
    log_print(f"Log saved to: {log_file}")
    return log_file

def main():
    log_print("=== DVP2-Linux64 Module Installation Script ===")
    log_print(f"Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    log_print(f"Current working directory: {os.getcwd()}")
    
    # 1. Check if install.sh exists in current directory
    install_script = "install.sh"
    if not os.path.isfile(install_script):
        log_print(f"Error: {install_script} not found in current directory")
        log_print("Please ensure the script is running in the DVP2-Linux64 directory and install.sh exists")
        modules_status = {'dt_pcie': False, 'dsfilter': False}
        modules_source = {}
        modules_detection = {}
        save_log(False, modules_status, modules_source, modules_detection, {})
        sys.exit(1)
    log_print(f"Found install.sh: {os.path.abspath(install_script)}")
    
    # 2. Unload existing modules
    log_print("\n--- Checking and unloading loaded modules ---")
    for mod in ['dt_pcie', 'dsfilter']:
        loaded, _ = check_module_status(mod)
        if loaded:
            log_print(f"Found existing module: {mod}")
            unload_module(mod)
            time.sleep(0.5)
        else:
            log_print(f"Module {mod} is not loaded")
    
    # 3. Execute install.sh
    log_print("\n--- Executing install.sh ---")
    log_print("Running install.sh with sudo, may require password...")
    install_ret = 0
    try:
        proc = subprocess.Popen(['sudo', 'bash', install_script],
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, bufsize=1)
        for line in proc.stdout:
            log_print(line.rstrip())
        install_ret = proc.wait()
    except Exception as e:
        log_print(f"Exception during install.sh execution: {e}")
        install_ret = -1
    
    if install_ret == 0:
        log_print("\ninstall.sh executed successfully")
    else:
        log_print(f"\ninstall.sh failed with return code: {install_ret}")
    
    # 4. First check of module status
    log_print("\n--- Initial module status check ---")
    modules_success = {}
    modules_detection = {}
    for mod in ['dt_pcie', 'dsfilter']:
        success, detection = check_module_status(mod)
        modules_success[mod] = success
        modules_detection[mod] = detection
        if success:
            log_print(f"✓ Module {mod}: SUCCESS (detection method: {detection})")
        else:
            log_print(f"✗ Module {mod}: FAILED (not detected)")
    
    # 5. Record sources and errors
    modules_source = {}
    make_errors = {}
    final_success = modules_success.copy()
    final_detection = modules_detection.copy()
    
    # 6. Attempt sudo make recovery for unsuccessful modules
    for mod in ['dt_pcie', 'dsfilter']:
        if not final_success.get(mod, False):
            success, error, detection = try_install_module_via_make(mod)
            if success:
                final_success[mod] = True
                final_detection[mod] = detection
                modules_source[mod] = "sudo make"
                log_print(f"✓ Module {mod} succeeded via sudo make (detection method: {detection})")
            else:
                modules_source[mod] = "FAILED"
                make_errors[mod] = error
                log_print(f"✗ Module {mod} still failed after sudo make")
        else:
            modules_source[mod] = "install.sh"
    
    # 7. Overall success condition: both modules ultimately successful
    overall_success = final_success.get('dt_pcie', False) and final_success.get('dsfilter', False)
    
    # 8. Save log
    save_log(overall_success, final_success, modules_source, final_detection, make_errors)
    
    # 9. Output final result
    log_print("\n" + "=" * 60)
    if overall_success:
        log_print("✓✓✓ All modules installed successfully ✓✓✓")
        for mod in ['dt_pcie', 'dsfilter']:
            detection = final_detection.get(mod, "unknown")
            if detection == "ko_file":
                log_print(f"  {mod}: SUCCESS (Source: {modules_source.get(mod, 'unknown')}, Note: Only .ko file detected, may require reboot to load into kernel)")
            else:
                log_print(f"  {mod}: SUCCESS (Source: {modules_source.get(mod, 'unknown')}, Detection method: {detection})")
    else:
        log_print("✗✗✗ Module installation failed ✗✗✗")
        failed = [mod for mod in ['dt_pcie', 'dsfilter'] if not final_success.get(mod, False)]
        log_print(f"Failed modules: {', '.join(failed)}")
    log_print(f"End time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    sys.exit(0 if overall_success else 1)

if __name__ == "__main__":
    main()