#include <iostream>
#include <math.h>

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"
#include "drim4hls.h"
#include "fast_float.h"

#include <mc_scverify.h>
#include <ac_int.h>

typedef ffp32 T;

class Top: public sc_module {
    public:

    CCS_DESIGN(drim4hls) CCS_INIT_S1(m_dut);

    sc_clock clk;
    SC_SIG(bool, rst);

    // End of simulation signal.
    #pragma hls_direct_input
    sc_signal < bool > CCS_INIT_S1(program_end);

    // Instruction counters
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(j_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(b_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(m_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(o_icount);

    /* The testbench, DUT, IMEM and DMEM modules. */
    Connections::Combinational < imem_out_t > CCS_INIT_S1(imem2de_ch);
    Connections::Combinational < imem_in_t > CCS_INIT_S1(fe2imem_ch);

    Connections::Combinational < dmem_out_t > CCS_INIT_S1(dmem2wb_ch);
    Connections::Combinational < dmem_in_t > CCS_INIT_S1(wb2dmem_ch);

    ac_int < XLEN, false > imem[ICACHE_SIZE];

    imem_out_t imem_dout;
    imem_in_t imem_din;

    ac_int < XLEN, false > dmem[DCACHE_SIZE];

    dmem_out_t dmem_dout;
    dmem_in_t dmem_din;
    
    int wait_stalls;

    const std::string testing_program;

    SC_CTOR(Top);
    Top(const sc_module_name &name, const std::string &testing_program): 
    clk("clk", 10, SC_NS, 5, 0, SC_NS, true),
    m_dut("drim4hls"),
    testing_program(testing_program) {
        
        std::cout << "PROVA" << std::endl;
        Connections::set_sim_clk( & clk);
        std::cout << "PROVA1" << std::endl;

        // Connect the design module
        m_dut.clk(clk);
        m_dut.rst(rst);
        m_dut.program_end(program_end);

        m_dut.icount(icount);
        m_dut.j_icount(j_icount);
        m_dut.b_icount(b_icount);
        m_dut.m_icount(m_icount);
        m_dut.o_icount(o_icount);

        m_dut.imem2de_data(imem2de_ch);
        m_dut.fe2imem_data(fe2imem_ch);
        m_dut.dmem2wb_data(dmem2wb_ch);
        m_dut.wb2dmem_data(wb2dmem_ch);

        SC_CTHREAD(run, clk);
        std::cout << "PROVA" << std::endl;

        SC_THREAD(imemory_th);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst, false);
    std::cout << "PROVA" << std::endl;

        SC_THREAD(dmemory_th);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst, false);
    std::cout << "PROVA2" << std::endl;
    }

    void imemory_th() {
        IMEM_RST: {
            imem2de_ch.ResetWrite();
            fe2imem_ch.ResetRead();

            wait();
        }
        IMEM_BODY: while (true) {
            imem_din = fe2imem_ch.Pop();

            sc_uint < XLEN > addr = imem_din.instr_addr.to_uint();
			//std::cout << "fetch instr " << endl;
			
			unsigned int offset_lenght = pow(2 , ICACHE_OFFSET_WIDTH);
			            
            for (int i = 0; i < offset_lenght; i++) {
				if (ICACHE_OFFSET_WIDTH) {
					addr.range(ICACHE_OFFSET_WIDTH - 1, 0) = (sc_uint < ICACHE_OFFSET_WIDTH >) i;                        
                }
				//std::cout << "imem addr= " << addr << endl;

                imem_dout.instr_data.set_slc(i*XLEN, imem[addr]);
                //std::cout << "imem[" << addr << "]=" << imem[addr] << endl;
			}

			
            unsigned int random_stalls = 1;
            wait(random_stalls);

            imem2de_ch.Push(imem_dout);
            wait();
        }

    }

    void dmemory_th() {
        DMEM_RST: {
            wb2dmem_ch.ResetRead();
            dmem2wb_ch.ResetWrite();
			wait_stalls = 0;
            wait();
        }
        DMEM_BODY: while (true) {
            dmem_din = wb2dmem_ch.Pop();

			sc_uint < XLEN > addr = dmem_din.data_addr.to_uint();
			sc_uint < XLEN > write_addr = dmem_din.write_addr.to_uint();
			
			unsigned int addr_lenght = DCACHE_TAG_WIDTH + DCACHE_INDEX_WIDTH;
            unsigned int offset_lenght = pow(2 , DCACHE_OFFSET_WIDTH);
            sc_uint < DCACHE_INDEX_WIDTH > index = addr.range(DCACHE_INDEX_WIDTH + DCACHE_OFFSET_WIDTH - 1, DCACHE_OFFSET_WIDTH);
            
            unsigned int random_stalls = 15;
            wait_stalls += random_stalls;
            wait(random_stalls);
             
            if (dmem_din.read_en) {
				//std::cout << "dmem read" << endl;

                for (int i = 0; i < offset_lenght; i++) {
                    if (DCACHE_OFFSET_WIDTH) {
                        addr.range(DCACHE_OFFSET_WIDTH - 1, 0) = (sc_uint < DCACHE_OFFSET_WIDTH >) i;                        
                    }
                    //std::cout << "dmem addr= " << addr << endl;

                    dmem_dout.data_out.set_slc(i*XLEN, dmem[addr]);
                    //std::cout << "dmem[" << addr << "]=" << dmem[addr] << endl;
                }
                
                dmem2wb_ch.Push(dmem_dout);
            } 
            if (dmem_din.write_en) {
				//std::cout << "dmem write" << endl;
                
                for (int i = 0; i < offset_lenght; i++) {
                    if (DCACHE_OFFSET_WIDTH) {
                        write_addr.range(DCACHE_OFFSET_WIDTH - 1, 0) = (sc_uint < DCACHE_OFFSET_WIDTH >) i;
                    }
                    //std::cout << "dmem addr= " << write_addr << endl;
                    dmem[write_addr] = dmem_din.data_in.slc<XLEN>(i*XLEN);
                    //std::cout << "dmem[" << write_addr << "]=" << dmem[write_addr] << endl;
                }
            }

            // REMOVE
    std::cout << "PROVA" << std::endl;
            wait();
        }

    }

    void run() {

        std::cout << "PROVA" << std::endl;
        std::ifstream load_program;
        load_program.open(testing_program, std::ifstream:: in );
        unsigned index;
        unsigned address;
        unsigned data;
        
        while (load_program >> std::hex >> address) {

            index = address >> 2;
            if (index >= ICACHE_SIZE) {
                SC_REPORT_ERROR(sc_object::name(), "Program larger than memory size.");
                sc_stop();
                return;
            }
            load_program >> data;
            //load_program >> std::hex >> imem[index];
            imem[index] = (ac_int<32, false>) data;
            std::cout << "imem[" << index << "]=" << imem[index] << endl;
            dmem[index] = imem[index];
        }

        load_program.close();

        std::cout << "PROVA" << std::endl;
        rst.write(0);
        wait(5);
        std::cout << "PROVA" << std::endl;
        rst.write(1);
        wait();
        std::cout << "PROVA" << std::endl;

        do {
            wait();
        } while (!program_end.read());
        wait(5);
        
        sc_stop();
        int dmem_index;
        for (dmem_index = 0; dmem_index < 600; dmem_index++) {
			T fast_float = dmem[dmem_index];
			float float_ieee = fast_float.to_float();
            std::cout << "dmem[" << dmem_index << "]=" << float_ieee << endl;
            //std::cout << "dmem[" << dmem_index << "]=" << dmem[dmem_index] << endl;
        }
		std::cout << "wait stalls " << wait_stalls << endl;
        long icount_end, j_icount_end, b_icount_end, m_icount_end, o_icount_end, pre_b_icount_end;

        icount_end = icount.read();
        j_icount_end = j_icount.read();
        b_icount_end = b_icount.read();
        m_icount_end = m_icount.read();
        o_icount_end = o_icount.read();

        SC_REPORT_INFO(sc_object::name(), "Program complete.");

        std::cout << "INSTR TOT: " << icount_end << std::endl;
        std::cout << "   JUMP  : " << j_icount_end << std::endl;
        std::cout << "   BRANCH: " << b_icount_end << std::endl;
        std::cout << "   MEM   : " << m_icount_end << std::endl;
        std::cout << "   OTHER : " << o_icount_end << std::endl;

    }

};

int sc_main(int argc, char * argv[]) {

    // if (argc == 1) {
    //     std::cerr << "Usage: " << argv[0] << " <testing_program>" << std::endl;
    //     std::cerr << "where:  <testing_program> - path to .txt file of the testing program" << std::endl;
    //     return -1;
    // }

    //std::string testing_program = argv[1];
    // USE IN QUESTASIM
    std::string testing_program = "./examples/binary_search/hello.txt";

    std::cout << "PROVA" << std::endl;
    Top top("top", testing_program);
    std::cout << "PROVA PRE" << std::endl;
    sc_start();
    return 0;
}
