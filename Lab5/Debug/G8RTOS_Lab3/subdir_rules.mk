################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
G8RTOS_Lab3/G8RTOS_CriticalSection.obj: C:/Users/Alex\ Perera/Desktop/uP2/Libraries/G8RTOS_Lab3/G8RTOS_CriticalSection.s $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/G8RTOS_Lab3" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/DriverLib" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/inc" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Alex Perera/Desktop/uP2/CCS_uP2/Lab4" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="G8RTOS_Lab3/G8RTOS_CriticalSection.d" --obj_directory="G8RTOS_Lab3" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

G8RTOS_Lab3/G8RTOS_FIFO.obj: C:/Users/Alex\ Perera/Desktop/uP2/Libraries/G8RTOS_Lab3/G8RTOS_FIFO.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/G8RTOS_Lab3" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/DriverLib" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/inc" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Alex Perera/Desktop/uP2/CCS_uP2/Lab4" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="G8RTOS_Lab3/G8RTOS_FIFO.d" --obj_directory="G8RTOS_Lab3" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

G8RTOS_Lab3/G8RTOS_Scheduler.obj: C:/Users/Alex\ Perera/Desktop/uP2/Libraries/G8RTOS_Lab3/G8RTOS_Scheduler.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/G8RTOS_Lab3" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/DriverLib" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/inc" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Alex Perera/Desktop/uP2/CCS_uP2/Lab4" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="G8RTOS_Lab3/G8RTOS_Scheduler.d" --obj_directory="G8RTOS_Lab3" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

G8RTOS_Lab3/G8RTOS_SchedulerASM.obj: C:/Users/Alex\ Perera/Desktop/uP2/Libraries/G8RTOS_Lab3/G8RTOS_SchedulerASM.s $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/G8RTOS_Lab3" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/DriverLib" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/inc" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Alex Perera/Desktop/uP2/CCS_uP2/Lab4" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="G8RTOS_Lab3/G8RTOS_SchedulerASM.d" --obj_directory="G8RTOS_Lab3" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

G8RTOS_Lab3/G8RTOS_Semaphores.obj: C:/Users/Alex\ Perera/Desktop/uP2/Libraries/G8RTOS_Lab3/G8RTOS_Semaphores.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/G8RTOS_Lab3" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/DriverLib" --include_path="C:/Users/Alex Perera/Desktop/uP2/Libraries/BoardSupportPackage/inc" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Alex Perera/Desktop/uP2/CCS_uP2/Lab4" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="G8RTOS_Lab3/G8RTOS_Semaphores.d" --obj_directory="G8RTOS_Lab3" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


