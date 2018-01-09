################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/digitrecognizer.cpp \
../src/puzzle.cpp 

OBJS += \
./src/digitrecognizer.o \
./src/puzzle.o 

CPP_DEPS += \
./src/digitrecognizer.d \
./src/puzzle.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/Cellar/opencv/ -O0 -g3 -std=c++11 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


