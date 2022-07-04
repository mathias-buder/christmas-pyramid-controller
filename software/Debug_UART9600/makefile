################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./System.obj" \
"./main.obj" \
"./startup_ccs.obj" \
"./Driver/ACT.obj" \
"./Driver/DS3231.obj" \
"./Driver/HCSR501.obj" \
"./Driver/HMTRP.obj" \
"./Driver/KY040.obj" \
"./Driver/LED.obj" \
"./Driver/NHD0216K3Z.obj" \
"./Driver/Timer.obj" \
"./Driver/VS10xx.obj" \
"./Driver/fatfs/cc932.obj" \
"./Driver/fatfs/ff.obj" \
"./Driver/fatfs/mmc-ek-tm4c123gxl.obj" \
"./Manager/AudioManager.obj" \
"./Manager/ConsoleManager.obj" \
"./Manager/DisplayManager.obj" \
"./Manager/FileManager.obj" \
"./Manager/TaskManager.obj" \
"./utils/cmdline.obj" \
"./utils/uartstdio.obj" \
"./utils/ustdlib.obj" \
"../BA-PYCO-FW.cmd" \
$(GEN_CMDS__FLAG) \
-l"C:/ti/TivaWare_C_Series-2.1.4.178/driverlib/ccs/Debug/driverlib.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Driver/subdir_vars.mk
-include Driver/fatfs/subdir_vars.mk
-include Manager/subdir_vars.mk
-include utils/subdir_vars.mk
-include subdir_rules.mk
-include Driver/subdir_rules.mk
-include Driver/fatfs/subdir_rules.mk
-include Manager/subdir_rules.mk
-include utils/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
BA-PYCO-FW.out \

EXE_OUTPUTS__QUOTED += \
"BA-PYCO-FW.out" \

BIN_OUTPUTS += \
BA-PYCO-FW.bin \

BIN_OUTPUTS__QUOTED += \
"BA-PYCO-FW.bin" \


# All Target
all: BA-PYCO-FW.out

# Tool invocations
BA-PYCO-FW.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --define=ENABLE_LFN --define=UART_9600 --define=UART_BUFFERED --define=PART_TM4C123GH6PM --define=ccs="ccs" --define=TARGET_IS_TM4C123_RA3 --define=ccs --define=PART_LM4F232H5BB -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --ual -z -m"project_ccs.map" --heap_size=0 --stack_size=5096 -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/lib" -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="BA-PYCO-FW_linkInfo.xml" --rom_model -o "BA-PYCO-FW.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

BA-PYCO-FW.bin: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armhex" --ti_txt -o "BA-PYCO-FW.bin" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "System.obj" "main.obj" "startup_ccs.obj" "Driver\ACT.obj" "Driver\DS3231.obj" "Driver\HCSR501.obj" "Driver\HMTRP.obj" "Driver\KY040.obj" "Driver\LED.obj" "Driver\NHD0216K3Z.obj" "Driver\Timer.obj" "Driver\VS10xx.obj" "Driver\fatfs\cc932.obj" "Driver\fatfs\ff.obj" "Driver\fatfs\mmc-ek-tm4c123gxl.obj" "Manager\AudioManager.obj" "Manager\ConsoleManager.obj" "Manager\DisplayManager.obj" "Manager\FileManager.obj" "Manager\TaskManager.obj" "utils\cmdline.obj" "utils\uartstdio.obj" "utils\ustdlib.obj" 
	-$(RM) "System.d" "main.d" "startup_ccs.d" "Driver\ACT.d" "Driver\DS3231.d" "Driver\HCSR501.d" "Driver\HMTRP.d" "Driver\KY040.d" "Driver\LED.d" "Driver\NHD0216K3Z.d" "Driver\Timer.d" "Driver\VS10xx.d" "Driver\fatfs\cc932.d" "Driver\fatfs\ff.d" "Driver\fatfs\mmc-ek-tm4c123gxl.d" "Manager\AudioManager.d" "Manager\ConsoleManager.d" "Manager\DisplayManager.d" "Manager\FileManager.d" "Manager\TaskManager.d" "utils\cmdline.d" "utils\uartstdio.d" "utils\ustdlib.d" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-"C:/ti/ccsv7/utils/tiobj2bin/tiobj2bin" "BA-PYCO-FW.out" "BA-PYCO-FW.bin" "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armofd" "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armhex" "C:/ti/ccsv7/utils/tiobj2bin/mkhex4bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

