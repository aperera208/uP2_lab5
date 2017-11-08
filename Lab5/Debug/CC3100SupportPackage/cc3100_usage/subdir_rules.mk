################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
CC3100SupportPackage/cc3100_usage/cc3100_usage.obj: C:/Users/Alex\ Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/cc3100_usage/cc3100_usage.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/G8RTOS_Lab5" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/Lab5_gameh" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/simplelink/include" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/BoardSupportPackage/DriverLib" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/BoardSupportPackage/inc" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/BoardSupportPackage/src" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/simplelink/source" --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/board" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/cc3100_usage" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/SL_Common" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/spi_cc3100" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/CC3100SupportPackage/simplelink/include" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Alex Perera/Desktop/uP2/uP2_lab5/Lab5" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.4.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs -g --c99 --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="CC3100SupportPackage/cc3100_usage/cc3100_usage.d" --obj_directory="CC3100SupportPackage/cc3100_usage" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


