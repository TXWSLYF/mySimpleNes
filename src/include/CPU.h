#ifndef CPU_H
#define CPU_H

#include <string>
#include <vector>

namespace mysn
{
    using Byte = std::uint8_t;
    using Address = std::uint16_t;
    using DobuleByte = std::uint16_t;

    // 寻址模式，指示 CPU 该如何处理操作码的后 1~2 个字节（Byte）
    enum AddressingMode
    {
        Accumulator,
        Immediate,
        Relative,
        ZeroPage,
        ZeroPage_X,
        ZeroPage_Y,
        Absolute,
        Absolute_X,
        Absolute_Y,
        Indirect,
        Indirect_X,
        Indirect_Y,
        NoneAddressing,
    };

    /// # Status Register (P) http://wiki.nesdev.com/w/index.php/Status_flags
    ///
    ///  7 6 5 4 3 2 1 0
    ///  N V _ B D I Z C
    ///  | |   | | | | +--- Carry Flag
    ///  | |   | | | +----- Zero Flag
    ///  | |   | | +------- Interrupt Disable
    ///  | |   | +--------- Decimal Mode (not used on NES)
    ///  | |   +----------- Break Command
    ///  | +--------------- Overflow Flag
    ///  +----------------- Negative Flag
    ///
    enum CpuFlags
    {
        Carry = 0b00000001,
        Zero = 0b00000010,
        Interrupt_Disable = 0b00000100,
        Decimal_Mode = 0b00001000,
        Break = 0b00010000,
        Break2 = 0b00100000,
        Overflow = 0b01000000,
        Negative = 0b10000000,
    };

    class CPU
    {
    private:
        std::vector<Byte> memory;

        void update_zero_and_negative_flags(Byte result);

        void adc(AddressingMode mode);
        void i_and(AddressingMode mode);
        void i_asl(AddressingMode mode);
        void i_asl_accumulator();
        void branch(bool condition);
        void bit(AddressingMode mode);
        void compare(AddressingMode mode, Byte compare_with);

        void dec(AddressingMode mode);
        void dex();
        void dey();

        void eor(AddressingMode mode);

        void inc(AddressingMode mode);

        void lda(AddressingMode mode);
        void ldx(AddressingMode mode);
        void ldy(AddressingMode mode);
        void lsr_accumulator();
        void lsr(AddressingMode mode);
        void rol_accumulator();
        void rol(AddressingMode mode);
        void ror_accumulator();
        void ror(AddressingMode mode);
        void ora(AddressingMode mode);
        void pla();
        void sbc(AddressingMode mode);
        void sta(AddressingMode mode);
        void tax();
        void inx();
        void iny();
        void stack_push_u16(Address addr);
        void stack_push(Byte data);
        Byte stack_pop();
        Address stack_pop_u16();

        DobuleByte mem_read_u16(Address addr);
        void mem_write_u16(Address addr, DobuleByte data);
        void load(std::vector<Byte> &program);
        void run();
        void reset();

        // 获取操作数地址
        Address get_operand_address(AddressingMode mode);

    public:
        CPU();

        Address program_counter;
        Byte register_a;
        Byte register_x;
        Byte register_y;
        Byte stack_pointer;

        // Status flags
        Byte status;

        void load_and_run(std::vector<Byte> &program);
        void mem_write(Address addr, Byte data);
        Byte mem_read(Address addr);

        void change_flag(CpuFlags flag, bool data);
        void set_flag(CpuFlags flag);
        void clear_flag(CpuFlags flag);
        bool contain_flag(CpuFlags flag);
    };
}

#endif // CPU_H
