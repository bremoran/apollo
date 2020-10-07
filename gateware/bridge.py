#!/usr/bin/env python3

from nmigen import *
from nmigen.cli import main


class OctalToQSPI(Elaboratable):
    def __init__(self, DDR=False):
        self.qCS      = Signal()
        self.qDATout  = Signal(4)
        self.oDATin   = Signal(8)
        self.oDATempty = Signal()


class QSPItoOctal(Elaboratable):
    def __init__(self, DDR=False):
        self.qCS      = Signal()
        self.qDATin   = Signal(4)
        self.oDAToutDV = Signal()
        self.oDATout  = Signal(8)

    def elaborate(self, platform):
        m = Module()
        dv = Signal()
        
        msn = Signal(4)

        m.d.sync += self.oDAToutDV.eq(~self.qCS & dv)
        
        with m.FSM() as fsm:
            with m.state("IDLE"):
                m.d.sync += dv.eq(0)
                with m.If(~self.qCS):
                    m.next = "MSN"
            with m.state("MSN"):
                with m.If(~self.qCS):
                    m.next = "LSN"
                with m.Else():
                    m.next = "IDLE"
                m.d.sync += msn.eq(self.qDATin)
            with m.state("LSN"):
                with m.If(~self.qCS):
                    m.next = "MSN"
                    m.d.sync += dv.eq(1)
                    m.d.sync += self.oDATout.eq(Cat(self.qDATin, msn))
                with m.Else():
                    m.state("IDLE")
                    m.d.sync += dv.eq(0)
        return m

class QSPIWishboneBridge(Elaboratable):
    def __init__(self, DDR=False):
        self.QDATI = Signal(4)
        self.QDATO = Signal(4)
        self.QCLK  = Signal()
        self.DQS   = Signal()
        self.CS    = Signal()

if __name__ == "__main__":
    q2o = QSPItoOctal()
    main(q2o, ports = [q2o.qCS, q2o.qDATin, q2o.oDATout, q2o.oDAToutDV])
