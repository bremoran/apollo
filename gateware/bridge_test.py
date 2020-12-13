from nmigen.back.pysim import Simulator, Settle


from bridge import QSPItoOctal

dut = QSPItoOctal()

def bench():
    yield dut.qCS.eq(1)
    for _ in range(30):
        yield
        assert not (yield dut.oDAToutDV)
    yield dut.qCS.eq(0)
    for y in range(4):
        x = y ^ 0xA5
        yield dut.qDATin.eq(x >> 4)
        yield
        # assert not (yield dut.oDAToutDV)
        yield dut.qDATin.eq(x & 0xF)
        yield
        yield Settle()
        assert x == (yield dut.oDATout)
        assert (yield dut.oDAToutDV)

sim = Simulator(dut)
sim.add_clock(1e-6)
sim.add_sync_process(bench)
with sim.write_vcd("qspi_bridge.vcd"):
    sim.run()