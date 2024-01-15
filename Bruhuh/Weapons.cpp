#include "Weapons.h"

bool WeaponComputer::next_instr()
{
	if (instructions.size() == 0) return false;

	while (instructions.at(programCounter).empty)
	{
		programCounter++;
		if (programCounter > instructions.size() - 1) return false;
		emptyCnt++;
		if (emptyCnt == instructions.size()) { emptyCnt = 0; return false; }
	}
	emptyCnt = 0;
	return true;
}

void WeaponComputer::set_register_values()
{
	for (auto& reg : registers)reg.clamp_value();
	for (auto& reg : bulletType.registers) reg.second.clamp_value();
}

void WeaponComputer::skip()
{
	skipCnt++;
	if (skipCnt > instructions.size())
	{
		skipCnt = 0;
		return;
	}
	if (next_instr())
		cycle();
	else
		end();
	skipCnt = 0;
}

void WeaponComputer::cycle()
{
	if (programCounter >= instructions.size()) programCounter = 0;
	WeaponInstruction& instruction = instructions.at(programCounter);

	if (instruction.empty) return;

	for (size_t i = 0; i < instruction.inputs.size(); i++)
	{
		bool isReg = instruction.isRegisterValue.at(i);

		if (isReg)
		{
			if (instruction.inputs.at(i) < 0) continue;

			instruction.values.at(i) = registers.at(instruction.inputs.at(i)).value;
		}
		else
		{
			instruction.values.at(i) = instruction.inputs.at(i);
		}
	}

	temperature += settings.heating;

	instruction.instructionFunction(*this, instruction);

	if (!skipCounterIncr)
		programCounter++;
	else
		skipCounterIncr = false;
}

void WeaponComputer::tick(size_t entityid)
{
	entity_id = entityid;
	int64_t timeSinceLast = (int64_t)(SDL_GetTicks() - clockLast) - (int64_t)delay;

	if (timeSinceLast > (int64_t)settings.clockSpeed)
	{
		delay = 0;
		clockLast = SDL_GetTicks();

		if (overheat && temperature > 0)
		{
			temperature = std::max(temperature - settings.cooling, 0.0);
			temperature--;
			return;
		}
		else
			overheat = false;
		booting = false;
		cycle();
		set_register_values();

		if (temperature > settings.maxTemp) 
		{
			overheat = true;
			end();
		}
		else if (!next_instr())
		{
			temperature = 0;
			end();
			next_instr();
		}
		
	}
}

void WeaponComputer::end()
{
	programCounter = 0;
	booting = true;
	delay += settings.bootSpeed;
	for (auto& reg : registers)
		reg.value = 0;
	for (auto& reg : bulletType.registers)
		reg.second.value = 0;
}
