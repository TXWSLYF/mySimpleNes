#include "CPU.h"
#include <iostream>
#include <CPUOpcodes.h>
#include <cmath>

namespace mysn
{
    CPU::CPU() : program_counter(0),
                 register_a(0),
                 register_x(0),
                 register_y(0),
                 stack_pointer(0xfd),
                 status(0),
                 memory(0xFFFF, 0){};

    // https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit
    void CPU::set_flag(CpuFlags flag)
    {
        change_flag(flag, true);
    }

    void CPU::clear_flag(CpuFlags flag)
    {
        change_flag(flag, false);
    }

    void CPU::change_flag(CpuFlags flag, bool data)
    {
        unsigned long newbit = !!data;
        int n = std::log2(int(flag));

        status ^= (-newbit ^ status) & (1UL << n);
    }

    bool CPU::contain_flag(CpuFlags flag)
    {
        return !!(status & flag);
    }

    void CPU::run()
    {
        while (true)
        {
            // 操作码
            auto code = mem_read(program_counter);
            ++program_counter;
            auto program_counter_state = program_counter;
            auto opcode = CPUOpcodes::CPU_OPS_CODES_MAP.find(code);

            // 判断操作码是否存在
            if (opcode == CPUOpcodes::CPU_OPS_CODES_MAP.end())
            {
                return;
            }

            auto mnemonic = (opcode->second).mnemonic;
            auto mode = (opcode->second).mode;
            auto len = (opcode->second).len;

            switch (mnemonic)
            {
            case CPUOpcodeMnemonics::ADC:
            {
                adc(mode);
                break;
            }

            case CPUOpcodeMnemonics::AND:
            {
                i_and(mode);
                break;
            }

            case CPUOpcodeMnemonics::ASL:
            {
                if (mode == AddressingMode::Accumulator)
                {
                    i_asl_accumulator();
                }
                else
                {

                    i_asl(mode);
                }
                break;
            }

            case CPUOpcodeMnemonics::BCC:
            {
                branch(!contain_flag(CpuFlags::Carry));
                break;
            }

            case CPUOpcodeMnemonics::BCS:
            {
                branch(contain_flag(CpuFlags::Carry));
                break;
            }

            case CPUOpcodeMnemonics::BEQ:
            {
                branch(contain_flag(CpuFlags::Zero));
                break;
            }

            case CPUOpcodeMnemonics::BIT:
            {
                bit(mode);
                break;
            }

            case CPUOpcodeMnemonics::BMI:
            {
                branch(contain_flag(CpuFlags::Negative));
                break;
            }

            case CPUOpcodeMnemonics::BNE:
            {
                branch(!contain_flag(CpuFlags::Zero));
                break;
            }

            case CPUOpcodeMnemonics::BPL:
            {
                branch(!contain_flag(CpuFlags::Negative));
                break;
            }

            case CPUOpcodeMnemonics::BVC:
            {
                branch(!contain_flag(CpuFlags::Overflow));
                break;
            }

            case CPUOpcodeMnemonics::BVS:
            {
                branch(contain_flag(CpuFlags::Overflow));
                break;
            }

            case CPUOpcodeMnemonics::CLC:
            {
                clear_flag(CpuFlags::Carry);
                break;
            }

            case CPUOpcodeMnemonics::CLD:
            {
                clear_flag(CpuFlags::Decimal_Mode);
                break;
            }

            case CPUOpcodeMnemonics::CLI:
            {
                clear_flag(CpuFlags::Interrupt_Disable);
                break;
            }

            case CPUOpcodeMnemonics::CLV:
            {
                clear_flag(CpuFlags::Overflow);
                break;
            }

            case CPUOpcodeMnemonics::CMP:
            {
                compare(mode, register_a);
                break;
            }

            case CPUOpcodeMnemonics::CPX:
            {
                compare(mode, register_x);
                break;
            }

            case CPUOpcodeMnemonics::CPY:
            {
                compare(mode, register_y);
                break;
            }

            case CPUOpcodeMnemonics::DEC:
            {
                dec(mode);
                break;
            }

            case CPUOpcodeMnemonics::DEX:
            {
                dex();
                break;
            }

            case CPUOpcodeMnemonics::DEY:
            {
                dey();
                break;
            }

            case CPUOpcodeMnemonics::EOR:
            {
                eor(mode);
                break;
            }

            case CPUOpcodeMnemonics::INC:
            {
                inc(mode);
                break;
            }

            case CPUOpcodeMnemonics::INX:
            {
                inx();
                break;
            }

            case CPUOpcodeMnemonics::INY:
            {
                iny();
                break;
            }

            case CPUOpcodeMnemonics::JMP:
            {
                if (mode == AddressingMode::Absolute)
                {
                    auto addr = mem_read_u16(program_counter);
                    program_counter = addr;
                }
                else if (mode == AddressingMode::Indirect)
                {
                    Address location = mem_read_u16(program_counter);
                    //6502 has a bug such that the when the vector of anindirect address begins at the last byte of a page,
                    //the second byte is fetched from the beginning of that page rather than the beginning of the next
                    //Recreating here:
                    Address Page = location & 0xff00;
                    program_counter = mem_read(location) |
                                      mem_read(Page | ((location + 1) & 0xff)) << 8;
                }

                break;
            }

            case CPUOpcodeMnemonics::JSR:
            {
                stack_push_u16(program_counter + 1);
                auto target_address = mem_read_u16(program_counter);
                program_counter = target_address;
            }

            case CPUOpcodeMnemonics::LDA:
            {
                lda(mode);
                break;
            }

            case CPUOpcodeMnemonics::LDX:
            {
                ldx(mode);
                break;
            }

            case CPUOpcodeMnemonics::LDY:
            {
                ldy(mode);
                break;
            }

            case CPUOpcodeMnemonics::LSR:
            {
                if (mode == AddressingMode::Accumulator)
                {
                    lsr_accumulator();
                }
                else
                {
                    lsr(mode);
                }
                break;
            }

            case CPUOpcodeMnemonics::NOP:
            {
                break;
            }

            case CPUOpcodeMnemonics::ORA:
            {
                ora(mode);
                break;
            }

            case CPUOpcodeMnemonics::PHA:
            {
                stack_push(register_a);
                break;
            }

            case CPUOpcodeMnemonics::PHP:
            {
                stack_push(status);
                set_flag(CpuFlags::Break);
                set_flag(CpuFlags::Break2);
                break;
            }

            case CPUOpcodeMnemonics::PLA:
            {
                pla();
                break;
            }

            case CPUOpcodeMnemonics::PLP:
            {
                status = stack_pop();
                clear_flag(CpuFlags::Break);
                set_flag(CpuFlags::Break2);
                break;
            }

            case CPUOpcodeMnemonics::ROL:
            {
                if (mode == AddressingMode::Accumulator)
                {
                    rol_accumulator();
                }
                else
                {
                    rol(mode);
                }
                break;
            }

            case CPUOpcodeMnemonics::ROR:
            {
                if (mode == AddressingMode::Accumulator)
                {
                    ror_accumulator();
                }
                else
                {
                    ror(mode);
                }
                break;
            }

            case CPUOpcodeMnemonics::RTI:
            {
                status = stack_pop();
                clear_flag(CpuFlags::Break);
                set_flag(CpuFlags::Break2);

                program_counter = stack_pop_u16();

                break;
            }

            case CPUOpcodeMnemonics::RTS:
            {
                program_counter = stack_pop_u16() + 1;
                break;
            }

            case CPUOpcodeMnemonics::SBC:
            {
                sbc(mode);
                break;
            }

            case CPUOpcodeMnemonics::SEC:
            {
                set_flag(CpuFlags::Carry);
                break;
            }

            case CPUOpcodeMnemonics::SED:
            {
                set_flag(CpuFlags::Decimal_Mode);
                break;
            }

            case CPUOpcodeMnemonics::SEI:
            {
                set_flag(CpuFlags::Interrupt_Disable);
                break;
            }

            case CPUOpcodeMnemonics::STA:
            {
                sta(mode);
                break;
            }

            case CPUOpcodeMnemonics::STX:
            {
                auto addr = get_operand_address(mode);
                mem_write(addr, register_x);
                break;
            }

            case CPUOpcodeMnemonics::STY:
            {
                auto addr = get_operand_address(mode);
                mem_write(addr, register_y);
                break;
            }

            case CPUOpcodeMnemonics::TAX:
            {
                tax();
                break;
            }

            case CPUOpcodeMnemonics::TAY:
            {
                register_y = register_a;
                update_zero_and_negative_flags(register_y);
                break;
            }

            case CPUOpcodeMnemonics::TSX:
            {
                register_x = stack_pointer;
                update_zero_and_negative_flags(register_x);
                break;
            }

            case CPUOpcodeMnemonics::TXA:
            {
                register_a = register_x;
                update_zero_and_negative_flags(register_a);
            }

            case CPUOpcodeMnemonics::TXS:
            {
                stack_pointer = register_x;
            }

            case CPUOpcodeMnemonics::TYA:
            {
                register_a = register_y;
                update_zero_and_negative_flags(register_a);
            }

            case CPUOpcodeMnemonics::BRK:
            {
                return;
            }
            }

            if (program_counter_state == program_counter)
            {
                program_counter += (len - 1);
            }
        }
    }

    void CPU::adc(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        std::uint16_t sum = register_a + value + (status & CpuFlags::Carry);

        if (sum & 0x100)
        {
            status = status | CpuFlags::Carry;
        }
        else
        {
            status = status & (~CpuFlags::Carry);
        }

        if ((register_a ^ sum) & (value ^ sum) & 0x80)
        {
            status = status | CpuFlags::Overflow;
        }
        else
        {
            status = status & (~CpuFlags::Overflow);
        }

        register_a = static_cast<Byte>(sum);
        update_zero_and_negative_flags(register_a);
    }

    void CPU::i_and(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        register_a = register_a & value;
        update_zero_and_negative_flags(register_a);
    }

    void CPU::i_asl_accumulator()
    {
        if (register_a & 0x80)
        {
            status = status | CpuFlags::Carry;
        }
        else
        {
            status = status & (~CpuFlags::Carry);
        }

        register_a = register_a << 1;
        update_zero_and_negative_flags(register_a);
    }

    void CPU::branch(bool condition)
    {
        if (condition)
        {
            int8_t jump = mem_read(program_counter);
            auto jump_addr = static_cast<Address>(program_counter + 1 + jump);

            program_counter = jump_addr;
        }
    }

    void CPU::bit(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        change_flag(CpuFlags::Zero, (register_a & value) == 0);
        change_flag(CpuFlags::Overflow, value & CpuFlags::Overflow);
        change_flag(CpuFlags::Negative, value & CpuFlags::Negative);
    }

    void CPU::compare(AddressingMode mode, Byte compare_with)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        std::uint16_t diff = compare_with - value;

        // fix: if the ninth bit is 1, the resulting number is negative => borrow => low carry
        change_flag(CpuFlags::Carry, !(diff & 0x100));
        update_zero_and_negative_flags(diff);
    }

    void CPU::dec(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr) - 1;

        mem_write(addr, value);
        update_zero_and_negative_flags(value);
    }

    void CPU::dex()
    {
        --register_x;
        update_zero_and_negative_flags(register_x);
    }

    void CPU::dey()
    {
        --register_y;
        update_zero_and_negative_flags(register_y);
    }

    void CPU::eor(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        register_a = value ^ register_a;
        update_zero_and_negative_flags(register_a);
    }

    void CPU::inc(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr) + 1;

        mem_write(addr, value);
        update_zero_and_negative_flags(value);
    }

    void CPU::i_asl(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        if (value & 0x80)
        {
            status = status | CpuFlags::Carry;
        }
        else
        {
            status = status & (~CpuFlags::Carry);
        }

        value = value << 1;
        mem_write(addr, value);
        update_zero_and_negative_flags(value);
    }

    void CPU::lda(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        register_a = value;
        update_zero_and_negative_flags(register_a);
    }

    void CPU::ldx(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        register_x = value;
        update_zero_and_negative_flags(register_x);
    }

    void CPU::ldy(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        register_y = value;
        update_zero_and_negative_flags(register_y);
    }

    void CPU::lsr_accumulator()
    {
        change_flag(CpuFlags::Carry, register_a & 1);
        register_a = register_a >> 1;
        update_zero_and_negative_flags(register_a);
    }

    void CPU::lsr(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        change_flag(CpuFlags::Carry, value & 1);
        value = value >> 1;
        mem_write(addr, value);
        update_zero_and_negative_flags(value);
    }

    void CPU::rol_accumulator()
    {
        auto old_carry = contain_flag(CpuFlags::Carry);
        change_flag(CpuFlags::Carry, (register_a >> 7) == 1);
        register_a = register_a << 1;

        if (old_carry)
        {
            register_a = register_a | 1;
        }
        update_zero_and_negative_flags(register_a);
    }

    void CPU::rol(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);
        auto old_carry = contain_flag(CpuFlags::Carry);

        change_flag(CpuFlags::Carry, (value >> 7) == 1);
        value = value << 1;
        if (old_carry)
        {
            value = value | 1;
        }
        mem_write(addr, value);
        update_zero_and_negative_flags(value);
    }

    void CPU::ror_accumulator()
    {
        auto old_carry = contain_flag(CpuFlags::Carry);

        change_flag(CpuFlags::Carry, (register_a & 1) == 1);
        register_a = register_a >> 1;

        if (old_carry)
        {
            register_a = register_a | 0b10000000;
        }

        update_zero_and_negative_flags(register_a);
    }

    void CPU::ror(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);
        auto old_carry = contain_flag(CpuFlags::Carry);

        change_flag(CpuFlags::Carry, (value & 1) == 1);
        value = value >> 1;

        if (old_carry)
        {
            value = value | 0b10000000;
        }

        mem_write(addr, value);
        update_zero_and_negative_flags(value);
    }

    void CPU::ora(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        register_a = value | register_a;
        update_zero_and_negative_flags(register_a);
    }

    void CPU::pla()
    {
        register_a = stack_pop();
        update_zero_and_negative_flags(register_a);
    }

    void CPU::sbc(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);
        auto value = mem_read(addr);

        auto diff = register_a - value - !contain_flag(CpuFlags::Carry);
        change_flag(CpuFlags::Carry, !(diff & 0x100));
        change_flag(CpuFlags::Overflow, (register_a ^ diff) & (~value ^ diff) & 0x80);
        register_a = diff;

        update_zero_and_negative_flags(diff);
    }

    void CPU::sta(AddressingMode mode)
    {
        auto addr = get_operand_address(mode);

        mem_write(addr, register_a);
    }

    void CPU::tax()
    {
        register_x = register_a;
        update_zero_and_negative_flags(register_x);
    }

    void CPU::inx()
    {
        ++register_x;
        update_zero_and_negative_flags(register_x);
    }

    void CPU::iny()
    {
        ++register_y;
        update_zero_and_negative_flags(register_y);
    }

    void CPU::stack_push_u16(Address addr)
    {
        stack_push(static_cast<Byte>((addr) >> 8));
        stack_push(static_cast<Byte>(addr));
    };

    void CPU::stack_push(Byte data)
    {
        mem_write(0x100 | stack_pointer, data);
        --stack_pointer; //Hardware stacks grow downward!
    }

    Byte CPU::stack_pop()
    {
        return mem_read(0x100 | ++stack_pointer);
    }

    Address CPU::stack_pop_u16()
    {
        auto lo = static_cast<Address>(stack_pop());
        auto hi = static_cast<Address>(stack_pop());

        return hi << 8 | lo;
    }

    void CPU::update_zero_and_negative_flags(Byte result)
    {
        change_flag(CpuFlags::Zero, !result);
        change_flag(CpuFlags::Negative, result & CpuFlags::Negative);
    };

    Byte CPU::mem_read(Address addr)
    {
        return memory[addr];
    }

    void CPU::mem_write(Address addr, Byte data)
    {
        memory[addr] = data;
    }

    DobuleByte CPU::mem_read_u16(Address addr)
    {
        return mem_read(addr) | mem_read(addr + 1) << 8;
    }

    void CPU::mem_write_u16(Address addr, DobuleByte data)
    {
        Byte low = Byte(data & 0xff);
        Byte high = Byte(data >> 8);

        mem_write(addr, low);
        mem_write(addr + 1, high);
    }

    void CPU::load(std::vector<Byte> &program)
    {
        Address start = 0x8000;

        // 程序数据（Program ROM/PRG ROM），存储在插入的墨盒中（Cartridges），存储的是游戏的代码
        // 从内存地址的 0x8000 开始装载
        // TODO: 暂时不考虑溢出情况
        for (auto i : program)
        {
            memory[start] = i;
            ++start;
        }

        mem_write_u16(0xFFFC, 0x8000);
    }

    void CPU::reset()
    {
        register_a = 0;
        register_x = 0;
        register_y = 0;
        status = 0;

        program_counter = mem_read_u16(0xFFFC);
    }

    void CPU::load_and_run(std::vector<Byte> &program)
    {
        load(program);
        reset();
        run();
    }

    Address CPU::get_operand_address(AddressingMode mode)
    {
        switch (mode)
        {
        case AddressingMode::Accumulator:
        {
            abort();
        };

        case AddressingMode::Relative:
        {
            abort();
        };

        case AddressingMode::Immediate:
        {
            return program_counter;
        }

        case AddressingMode::ZeroPage:
        {

            return Address(mem_read(program_counter));
        }

        case AddressingMode::Absolute:
        {

            return mem_read_u16(program_counter);
        }

        case AddressingMode::ZeroPage_X:
        {
            auto pos = mem_read(program_counter);
            Address addr = Address(pos) + Address(register_x);

            return addr;
        }

        case AddressingMode::ZeroPage_Y:
        {
            auto pos = mem_read(program_counter);
            Address addr = Address(pos) + Address(register_y);

            return addr;

            break;
        }

        case AddressingMode::Absolute_X:
        {
            auto base = mem_read_u16(program_counter);
            Address addr = base + register_x;

            return addr;
        }

        case AddressingMode::Absolute_Y:
        {
            auto base = mem_read_u16(program_counter);
            Address addr = base + register_y;

            return addr;
        }

        case AddressingMode::Indirect:
        {
            abort();
        }

        case AddressingMode::Indirect_X:
        {
            auto base = mem_read(program_counter);
            auto ptr = Byte(base + register_x);
            DobuleByte lo = DobuleByte(mem_read(ptr));
            DobuleByte hi = DobuleByte(Byte(ptr + 1));

            return lo | hi << 8;
        }

        case AddressingMode::Indirect_Y:
        {
            auto base = mem_read(program_counter);
            auto lo = mem_read(base);
            auto hi = mem_read(base + 1);
            auto deref_base = lo | hi << 8;
            auto deref = register_y + deref_base;

            return deref;
        }

        case AddressingMode::NoneAddressing:
        {
            std::cout << mode << "is not supported" << std::endl;
            abort();
        }
        }
    };

}