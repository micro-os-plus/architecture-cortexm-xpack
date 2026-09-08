import xml.etree.ElementTree as ET

BASE = '/tmp/claude-0/-home-claude/66138eca-62cf-5e0a-959c-5a69e5b4eed7/scratchpad/svd'

def mkfield(name, off, width, desc=None, access=None):
    f = ET.Element('field')
    ET.SubElement(f, 'name').text = name
    if desc:
        ET.SubElement(f, 'description').text = desc
    ET.SubElement(f, 'bitOffset').text = str(off)
    ET.SubElement(f, 'bitWidth').text = str(width)
    if access:
        ET.SubElement(f, 'access').text = access
    return f

def mkreg(name, offset, access, reset, fields, desc=None, size=32):
    r = ET.Element('register')
    ET.SubElement(r, 'name').text = name
    if desc:
        ET.SubElement(r, 'description').text = desc
    ET.SubElement(r, 'addressOffset').text = offset
    ET.SubElement(r, 'size').text = str(size)
    ET.SubElement(r, 'access').text = access
    ET.SubElement(r, 'resetValue').text = reset
    fl = ET.SubElement(r, 'fields')
    for f in fields:
        fl.append(f)
    return r

def mkregarray(name, offset, dim, dimincrement, access, reset, fields=None, desc=None):
    r = ET.Element('register')
    ET.SubElement(r, 'name').text = f'{name}%s'
    ET.SubElement(r, 'dim').text = str(dim)
    ET.SubElement(r, 'dimIncrement').text = dimincrement
    if desc:
        ET.SubElement(r, 'description').text = desc
    ET.SubElement(r, 'addressOffset').text = offset
    ET.SubElement(r, 'size').text = '32'
    ET.SubElement(r, 'access').text = access
    ET.SubElement(r, 'resetValue').text = reset
    if fields:
        fl = ET.SubElement(r, 'fields')
        for f in fields:
            fl.append(f)
    return r

def priofields():
    # 4 x 8-bit priority fields per 32-bit IPR word, full 8-bit width per
    # architecture -- only the top __NVIC_PRIO_BITS bits are implemented in
    # silicon (2-8 depending on device), unimplemented low bits read as 0.
    return [
        mkfield('PRI_N0', 0, 8, 'Priority, byte offset 0'),
        mkfield('PRI_N1', 8, 8, 'Priority, byte offset 1'),
        mkfield('PRI_N2', 16, 8, 'Priority, byte offset 2'),
        mkfield('PRI_N3', 24, 8, 'Priority, byte offset 3'),
    ]

def mk_peripheral(name, base_addr, desc, group=None, size='0x400'):
    p = ET.Element('peripheral')
    ET.SubElement(p, 'name').text = name
    ET.SubElement(p, 'description').text = desc
    ET.SubElement(p, 'groupName').text = group or name
    ET.SubElement(p, 'baseAddress').text = base_addr
    ab = ET.SubElement(p, 'addressBlock')
    ET.SubElement(ab, 'offset').text = '0x0'
    ET.SubElement(ab, 'size').text = size
    ET.SubElement(ab, 'usage').text = 'registers'
    regs = ET.SubElement(p, 'registers')
    return p, regs

# ---------------------------------------------------------------------------
# CoreDebug -- identical across v6-M/v7-M/v8-M (fixed ADIv5/CoreSight debug
# architecture, not part of the M-profile ISA). Verified against
# CMSIS_5 core_cm4.h defines.
# ---------------------------------------------------------------------------
def build_coredebug(v6m_note=False):
    p, regs = mk_peripheral('CoreDebug', '0xE000EDF0',
        'Core Debug Registers (DHCSR, DCRSR, DCRDR, DEMCR). Fixed ADIv5/CoreSight '
        'debug architecture -- identical on every Cortex-M core.' +
        (' On ARMv6-M these registers are only accessible over the external '
         'Debug Access Port (DAP) by a debugger/probe -- application/firmware '
         'code running on the processor cannot read or write them via normal '
         'load/store instructions (confirmed in CMSIS core_cm0.h/core_cm0plus.h: '
         '"Core Debug Registers ... are only accessible over DAP and not via '
         'processor"). On ARMv7-M and later they ARE accessible from firmware.'
         if v6m_note else ''),
        size='0x10')
    regs.append(mkreg('DHCSR', '0x0', 'read-write', '0x00000000', [
        mkfield('C_DEBUGEN', 0, 1), mkfield('C_HALT', 1, 1), mkfield('C_STEP', 2, 1),
        mkfield('C_MASKINTS', 3, 1), mkfield('C_SNAPSTALL', 5, 1),
        mkfield('S_REGRDY', 16, 1, access='read-only'),
        mkfield('S_HALT', 17, 1, access='read-only'),
        mkfield('S_SLEEP', 18, 1, access='read-only'),
        mkfield('S_LOCKUP', 19, 1, access='read-only'),
        mkfield('S_RETIRE_ST', 24, 1, access='read-only'),
        mkfield('S_RESET_ST', 25, 1, access='read-only'),
        mkfield('DBGKEY', 16, 16, 'Write 0xA05F to enable write access to bits [15:0]; reads as 0',
                access='write-only'),
    ], desc='Debug Halting Control and Status Register'))
    regs.append(mkreg('DCRSR', '0x4', 'write-only', '0x00000000', [
        mkfield('REGSEL', 0, 5), mkfield('REGWnR', 16, 1),
    ], desc='Debug Core Register Selector Register'))
    regs.append(mkreg('DCRDR', '0x8', 'read-write', '0x00000000', [
        mkfield('DBGTMP', 0, 32),
    ], desc='Debug Core Register Data Register'))
    regs.append(mkreg('DEMCR', '0xC', 'read-write', '0x00000000', [
        mkfield('VC_CORERESET', 0, 1, 'Reset Vector Catch'),
        mkfield('VC_MMERR', 4, 1, 'Debug trap on MemManage exception -- ARMv7-M and later only'),
        mkfield('VC_NOCPERR', 5, 1, 'Debug trap on UsageFault caused by no coprocessor -- ARMv7-M and later only'),
        mkfield('VC_CHKERR', 6, 1, 'Debug trap on UsageFault caused by checking error -- ARMv7-M and later only'),
        mkfield('VC_STATERR', 7, 1, 'Debug trap on UsageFault state error -- ARMv7-M and later only'),
        mkfield('VC_BUSERR', 8, 1, 'Debug trap on BusFault exception -- ARMv7-M and later only'),
        mkfield('VC_INTERR', 9, 1, 'Debug trap on interrupt/exception service errors -- ARMv7-M and later only'),
        mkfield('VC_HARDERR', 10, 1, 'Debug trap on HardFault exception'),
        mkfield('MON_EN', 16, 1, 'Debug Monitor enable -- ARMv7-M and later only'),
        mkfield('MON_PEND', 17, 1, 'Debug Monitor pend -- ARMv7-M and later only'),
        mkfield('MON_STEP', 18, 1, 'Debug Monitor step -- ARMv7-M and later only'),
        mkfield('MON_REQ', 19, 1, 'Debug Monitor request -- ARMv7-M and later only'),
        mkfield('TRCENA', 24, 1, 'Trace enable (DWT/ITM/ETM/TPIU) -- only meaningful where those blocks exist'),
    ], desc='Debug Exception and Monitor Control Register. On ARMv6-M only VC_CORERESET '
            'and TRCENA are architecturally defined; the fault-related VC_* bits and the '
            'monitor MON_* bits apply from ARMv7-M up (v6-M has no MemManage/BusFault/'
            'UsageFault/Debug-Monitor exception, so those control bits do not exist there '
            '-- included here for completeness/documentation, treat as reserved on v6-M).'))
    return p

# ---------------------------------------------------------------------------
# SysTick -- identical across the whole Cortex-M family.
# ---------------------------------------------------------------------------
def build_systick():
    p, regs = mk_peripheral('SysTick', '0xE000E010', 'SysTick Timer. Identical register set on every '
                             'Cortex-M core (ARMv6-M through current ARMv8-M).', size='0x10')
    regs.append(mkreg('CTRL', '0x0', 'read-write', '0x00000000', [
        mkfield('ENABLE', 0, 1), mkfield('TICKINT', 1, 1), mkfield('CLKSOURCE', 2, 1),
        mkfield('COUNTFLAG', 16, 1, access='read-only'),
    ], desc='SysTick Control and Status Register'))
    regs.append(mkreg('LOAD', '0x4', 'read-write', '0x00000000', [
        mkfield('RELOAD', 0, 24),
    ], desc='SysTick Reload Value Register'))
    regs.append(mkreg('VAL', '0x8', 'read-write', '0x00000000', [
        mkfield('CURRENT', 0, 24, access='read-write'),
    ], desc='SysTick Current Value Register. Any write clears it to 0 and clears COUNTFLAG.'))
    regs.append(mkreg('CALIB', '0xC', 'read-only', '0x00000000', [
        mkfield('TENMS', 0, 24), mkfield('SKEW', 30, 1), mkfield('NOREF', 31, 1),
    ], desc='SysTick Calibration Value Register'))
    return p

print('helpers loaded')

# ---------------------------------------------------------------------------
# NVIC
# ---------------------------------------------------------------------------
def build_nvic_v6m():
    p, regs = mk_peripheral('NVIC', '0xE000E100',
        'Nested Vectored Interrupt Controller. ARMv6-M caps external interrupts at '
        '32 (5-bit encoding), so exactly one ISER/ICER/ISPR/ICPR register and 8 IPR '
        'registers cover the whole architectural range -- true for every ARMv6-M '
        'device regardless of how many interrupt lines it actually implements '
        '(unimplemented lines simply read as 0 / writes ignored). No IABR and no '
        'STIR on ARMv6-M (both are ARMv7-M+ only).', size='0x320')
    regs.append(mkreg('ISER', '0x0', 'read-write', '0x00000000', [mkfield('SETENA', 0, 32)],
                       desc='Interrupt Set-Enable Register'))
    regs.append(mkreg('ICER', '0x80', 'read-write', '0x00000000', [mkfield('CLRENA', 0, 32)],
                       desc='Interrupt Clear-Enable Register'))
    regs.append(mkreg('ISPR', '0x100', 'read-write', '0x00000000', [mkfield('SETPEND', 0, 32)],
                       desc='Interrupt Set-Pending Register'))
    regs.append(mkreg('ICPR', '0x180', 'read-write', '0x00000000', [mkfield('CLRPEND', 0, 32)],
                       desc='Interrupt Clear-Pending Register'))
    for i in range(8):
        regs.append(mkreg(f'IPR{i}', hex(0x300 + i*4), 'read-write', '0x00000000', priofields(),
                           desc=f'Interrupt Priority Register {i} (covers IRQ{i*4}..IRQ{i*4+3})'))
    return p

def build_nvic_v7m():
    p, regs = mk_peripheral('NVIC', '0xE000E100',
        'Nested Vectored Interrupt Controller. ARMv7-M allows up to 240 external '
        'interrupts, so this is sized to the full architectural maximum: 8 banks of '
        'ISER/ICER/ISPR/ICPR/IABR and 60 IPR registers. A real device with fewer '
        'implemented interrupts still has this whole address range reserved by the '
        'architecture -- it just leaves the trailing registers unimplemented '
        '(reads as 0). This matches how CMSIS itself models NVIC_Type generically '
        '(ISER[8]/ICER[8]/ISPR[8]/ICPR[8]/IABR[8]/IP[240] in core_cm3.h/core_cm4.h) '
        '-- your MCU\'s own vendor SVD/header will only expose as many IRQn as it '
        'actually implements.', size='0xF04')
    for base, nm, desc in [(0x000, 'ISER', 'Interrupt Set-Enable Register'),
                            (0x080, 'ICER', 'Interrupt Clear-Enable Register'),
                            (0x100, 'ISPR', 'Interrupt Set-Pending Register'),
                            (0x180, 'ICPR', 'Interrupt Clear-Pending Register'),
                            (0x200, 'IABR', 'Interrupt Active Bit Register')]:
        fld = [mkfield('BITS', 0, 32)]
        if nm == 'IABR':
            for f in fld:
                pass
        acc = 'read-only' if nm == 'IABR' else 'read-write'
        regs.append(mkregarray(nm, hex(base), 8, '0x4', acc, '0x00000000',
                                fields=fld, desc=desc))
    regs.append(mkregarray('IPR', '0x300', 60, '0x4', 'read-write', '0x00000000',
                            fields=priofields(),
                            desc='Interrupt Priority Registers 0-59 (cover IRQ0..IRQ239)'))
    regs.append(mkreg('STIR', '0xE00', 'write-only', '0x00000000', [
        mkfield('INTID', 0, 9, 'Interrupt ID of the interrupt to trigger, 0-239'),
    ], desc='Software Trigger Interrupt Register. Only usable if SCB->CCR.USERSETMPEND '
            'allows unprivileged access and the device implements this register.'))
    return p

print('nvic builders loaded')

# ---------------------------------------------------------------------------
# SCB
# ---------------------------------------------------------------------------
def cpuid_fields():
    return [
        mkfield('Revision', 0, 4, 'Implementation/revision specific (e.g. r0p0 = 0x0)'),
        mkfield('PartNo', 4, 12, 'Implementation specific part number'),
        mkfield('Constant', 16, 4, 'Reads as 0xC for ARMv6-M, 0xF for ARMv7-M/ARMv8-M'),
        mkfield('Variant', 20, 4, 'Implementation specific variant number'),
        mkfield('Implementer', 24, 8, "0x41 = 'A' = ARM"),
    ]

def build_scb_v6m():
    p, regs = mk_peripheral('SCB', '0xE000ED00',
        'System Control Block, ARMv6-M register set. Verified against CMSIS_5 '
        'core_cm0.h/core_cm0plus.h defines. No SHPR1, no CFSR/HFSR/MMFAR/BFAR/AFSR '
        '(no MemManage/BusFault/UsageFault on v6-M), no AIRCR.PRIGROUP/VECTRESET, no '
        'ICSR.RETTOBASE (all ARMv7-M+ only). SHCSR exists but with only one bit '
        '(SVCALLPENDED) implemented. VTOR is present in the address map but is only '
        'wired up on implementations with __VTOR_PRESENT=1 (e.g. Cortex-M0+); on '
        'plain Cortex-M0 that offset is reserved. SHCSR and DFSR are only accessible '
        'over the DAP (debugger), not from application code -- see the CoreDebug '
        'peripheral\'s description.', size='0x40')
    regs.append(mkreg('CPUID', '0x0', 'read-only', '0x00000000', cpuid_fields(),
                       desc='CPUID Base Register. Reset value is implementation/revision specific.'))
    regs.append(mkreg('ICSR', '0x4', 'read-write', '0x00000000', [
        mkfield('VECTACTIVE', 0, 9, 'Active exception number'),
        mkfield('VECTPENDING', 12, 9, 'Pending exception number'),
        mkfield('ISRPENDING', 22, 1, access='read-only'),
        mkfield('ISRPREEMPT', 23, 1, 'Only meaningful with a debugger attached', access='read-only'),
        mkfield('PENDSTCLR', 25, 1, access='write-only'),
        mkfield('PENDSTSET', 26, 1),
        mkfield('PENDSVCLR', 27, 1, access='write-only'),
        mkfield('PENDSVSET', 28, 1),
        mkfield('NMIPENDSET', 31, 1),
    ], desc='Interrupt Control and State Register. No RETTOBASE (ARMv7-M+ only).'))
    regs.append(mkreg('VTOR', '0x8', 'read-write', '0x00000000', [
        mkfield('TBLOFF', 7, 25, 'Vector table base offset, 128-byte aligned'),
    ], desc='Vector Table Offset Register -- OPTIONAL: only present if the implementation sets '
            '__VTOR_PRESENT=1 (common on Cortex-M0+, absent on plain Cortex-M0). Reads as 0 / '
            'reserved otherwise.'))
    regs.append(mkreg('AIRCR', '0xC', 'read-write', '0x00000000', [
        mkfield('VECTCLRACTIVE', 1, 1, 'Reserved for debug use'),
        mkfield('SYSRESETREQ', 2, 1),
        mkfield('ENDIANESS', 15, 1, access='read-only'),
        mkfield('VECTKEY', 16, 16, 'Write 0x05FA to permit the write; reads as 0xFA05'),
    ], desc='Application Interrupt and Reset Control Register. No PRIGROUP or VECTRESET '
            '(both ARMv7-M+ only -- v6-M has no configurable priority grouping).'))
    regs.append(mkreg('SCR', '0x10', 'read-write', '0x00000000', [
        mkfield('SLEEPONEXIT', 1, 1), mkfield('SLEEPDEEP', 2, 1), mkfield('SEVONPEND', 4, 1),
    ], desc='System Control Register'))
    regs.append(mkreg('CCR', '0x14', 'read-write', '0x00000200', [
        mkfield('UNALIGN_TRP', 3, 1, 'Trap on unaligned word/halfword access'),
        mkfield('STKALIGN', 9, 1, 'Always reads as 1: 8-byte stack alignment on exception entry'),
    ], desc='Configuration and Control Register'))
    regs.append(mkreg('SHPR2', '0x1C', 'read-write', '0x00000000', [
        mkfield('PRI_11', 24, 8, 'Priority of exception 11, SVCall'),
    ], desc='System Handler Priority Register 2'))
    regs.append(mkreg('SHPR3', '0x20', 'read-write', '0x00000000', [
        mkfield('PRI_14', 16, 8, 'Priority of exception 14, PendSV'),
        mkfield('PRI_15', 24, 8, 'Priority of exception 15, SysTick'),
    ], desc='System Handler Priority Register 3'))
    regs.append(mkreg('SHCSR', '0x24', 'read-write', '0x00000000', [
        mkfield('SVCALLPENDED', 15, 1, 'The only bit implemented on ARMv6-M'),
    ], desc='System Handler Control and State Register. DAP-only access on ARMv6-M.'))
    return p

def build_scb_v7m():
    p, regs = mk_peripheral('SCB', '0xE000ED00',
        'System Control Block, ARMv7-M register set. Verified against CMSIS_5 '
        'core_cm3.h/core_cm4.h defines (both are identical at the SCB level -- the '
        'FPU-related extras live in a separate optional FPU block, not shown here, '
        'since plain Cortex-M3 has no FPU at all). PFR/DFR/ADR/MMFR/ISAR are read-only '
        'ID registers whose exact values are implementation specific.', size='0x90')
    regs.append(mkreg('CPUID', '0x0', 'read-only', '0x00000000', cpuid_fields(),
                       desc='CPUID Base Register. Reset value is implementation/revision specific.'))
    regs.append(mkreg('ICSR', '0x4', 'read-write', '0x00000000', [
        mkfield('VECTACTIVE', 0, 9), mkfield('RETTOBASE', 11, 1, access='read-only'),
        mkfield('VECTPENDING', 12, 9), mkfield('ISRPENDING', 22, 1, access='read-only'),
        mkfield('ISRPREEMPT', 23, 1, access='read-only'),
        mkfield('PENDSTCLR', 25, 1, access='write-only'), mkfield('PENDSTSET', 26, 1),
        mkfield('PENDSVCLR', 27, 1, access='write-only'), mkfield('PENDSVSET', 28, 1),
        mkfield('NMIPENDSET', 31, 1),
    ], desc='Interrupt Control and State Register'))
    regs.append(mkreg('VTOR', '0x8', 'read-write', '0x00000000', [
        mkfield('TBLOFF', 7, 25),
    ], desc='Vector Table Offset Register'))
    regs.append(mkreg('AIRCR', '0xC', 'read-write', '0x00000000', [
        mkfield('VECTRESET', 0, 1, 'Reserved for debug use'),
        mkfield('VECTCLRACTIVE', 1, 1, 'Reserved for debug use'),
        mkfield('SYSRESETREQ', 2, 1),
        mkfield('PRIGROUP', 8, 3, 'Interrupt priority grouping'),
        mkfield('ENDIANESS', 15, 1, access='read-only'),
        mkfield('VECTKEY', 16, 16, 'Write 0x05FA to permit the write; reads as 0xFA05'),
    ], desc='Application Interrupt and Reset Control Register'))
    regs.append(mkreg('SCR', '0x10', 'read-write', '0x00000000', [
        mkfield('SLEEPONEXIT', 1, 1), mkfield('SLEEPDEEP', 2, 1), mkfield('SEVONPEND', 4, 1),
    ], desc='System Control Register'))
    regs.append(mkreg('CCR', '0x14', 'read-write', '0x00000200', [
        mkfield('NONBASETHRDENA', 0, 1), mkfield('USERSETMPEND', 1, 1),
        mkfield('UNALIGN_TRP', 3, 1), mkfield('DIV_0_TRP', 4, 1),
        mkfield('BFHFNMIGN', 8, 1), mkfield('STKALIGN', 9, 1),
    ], desc='Configuration and Control Register'))
    regs.append(mkreg('SHPR1', '0x18', 'read-write', '0x00000000', [
        mkfield('PRI_4', 0, 8, 'MemManage fault'), mkfield('PRI_5', 8, 8, 'BusFault'),
        mkfield('PRI_6', 16, 8, 'UsageFault'),
    ], desc='System Handler Priority Register 1'))
    regs.append(mkreg('SHPR2', '0x1C', 'read-write', '0x00000000', [
        mkfield('PRI_11', 24, 8, 'SVCall'),
    ], desc='System Handler Priority Register 2'))
    regs.append(mkreg('SHPR3', '0x20', 'read-write', '0x00000000', [
        mkfield('PRI_14', 16, 8, 'PendSV'), mkfield('PRI_15', 24, 8, 'SysTick'),
    ], desc='System Handler Priority Register 3'))
    regs.append(mkreg('SHCSR', '0x24', 'read-write', '0x00000000', [
        mkfield('MEMFAULTACT', 0, 1), mkfield('BUSFAULTACT', 1, 1), mkfield('USGFAULTACT', 3, 1),
        mkfield('SVCALLACT', 7, 1), mkfield('MONITORACT', 8, 1), mkfield('PENDSVACT', 10, 1),
        mkfield('SYSTICKACT', 11, 1), mkfield('USGFAULTPENDED', 12, 1),
        mkfield('MEMFAULTPENDED', 13, 1), mkfield('BUSFAULTPENDED', 14, 1),
        mkfield('SVCALLPENDED', 15, 1), mkfield('MEMFAULTENA', 16, 1),
        mkfield('BUSFAULTENA', 17, 1), mkfield('USGFAULTENA', 18, 1),
    ], desc='System Handler Control and State Register'))
    regs.append(mkreg('CFSR', '0x28', 'read-write', '0x00000000', [
        mkfield('IACCVIOL', 0, 1, 'MMFSR'), mkfield('DACCVIOL', 1, 1, 'MMFSR'),
        mkfield('MUNSTKERR', 3, 1, 'MMFSR'), mkfield('MSTKERR', 4, 1, 'MMFSR'),
        mkfield('MMARVALID', 7, 1, 'MMFSR', access='read-only'),
        mkfield('IBUSERR', 8, 1, 'BFSR'), mkfield('PRECISERR', 9, 1, 'BFSR'),
        mkfield('IMPRECISERR', 10, 1, 'BFSR'), mkfield('UNSTKERR', 11, 1, 'BFSR'),
        mkfield('STKERR', 12, 1, 'BFSR'), mkfield('BFARVALID', 15, 1, 'BFSR', access='read-only'),
        mkfield('UNDEFINSTR', 16, 1, 'UFSR'), mkfield('INVSTATE', 17, 1, 'UFSR'),
        mkfield('INVPC', 18, 1, 'UFSR'), mkfield('NOCP', 19, 1, 'UFSR'),
        mkfield('UNALIGNED', 24, 1, 'UFSR'), mkfield('DIVBYZERO', 25, 1, 'UFSR'),
    ], desc='Configurable Fault Status Register = MMFSR[7:0] | BFSR[15:8] | UFSR[31:16]'))
    regs.append(mkreg('HFSR', '0x2C', 'read-write', '0x00000000', [
        mkfield('VECTTBL', 1, 1), mkfield('FORCED', 30, 1), mkfield('DEBUGEVT', 31, 1),
    ], desc='HardFault Status Register'))
    regs.append(mkreg('DFSR', '0x30', 'read-write', '0x00000000', [
        mkfield('HALTED', 0, 1), mkfield('BKPT', 1, 1), mkfield('DWTTRAP', 2, 1),
        mkfield('VCATCH', 3, 1), mkfield('EXTERNAL', 4, 1),
    ], desc='Debug Fault Status Register'))
    regs.append(mkreg('MMFAR', '0x34', 'read-write', '0x00000000', [mkfield('ADDRESS', 0, 32)],
                       desc='MemManage Fault Address Register, valid only when CFSR.MMARVALID is set'))
    regs.append(mkreg('BFAR', '0x38', 'read-write', '0x00000000', [mkfield('ADDRESS', 0, 32)],
                       desc='BusFault Address Register, valid only when CFSR.BFARVALID is set'))
    regs.append(mkreg('AFSR', '0x3C', 'read-write', '0x00000000', [mkfield('IMPDEF', 0, 32)],
                       desc='Auxiliary Fault Status Register, implementation defined'))
    regs.append(mkregarray('PFR', '0x40', 2, '0x4', 'read-only', '0x00000000',
                            desc='Processor Feature Register, implementation defined'))
    regs.append(mkreg('DFR', '0x48', 'read-only', '0x00000000', [], desc='Debug Feature Register, implementation defined'))
    regs.append(mkreg('ADR', '0x4C', 'read-only', '0x00000000', [], desc='Auxiliary Feature Register, implementation defined'))
    regs.append(mkregarray('MMFR', '0x50', 4, '0x4', 'read-only', '0x00000000',
                            desc='Memory Model Feature Register, implementation defined'))
    regs.append(mkregarray('ISAR', '0x60', 5, '0x4', 'read-only', '0x00000000',
                            desc='Instruction Set Attributes Register, implementation defined'))
    regs.append(mkreg('CPACR', '0x88', 'read-write', '0x00000000', [
        mkfield('CP10', 20, 2, 'FPU access privileges -- only meaningful if an FPU is implemented (Cortex-M4/M7)'),
        mkfield('CP11', 22, 2, 'FPU access privileges -- only meaningful if an FPU is implemented (Cortex-M4/M7)'),
    ], desc='Coprocessor Access Control Register. Present architecturally on every ARMv7-M '
            'core; only relevant when a coprocessor (typically the optional FPU on '
            'Cortex-M4/M7) is actually implemented -- reads as 0 / has no effect otherwise.'))
    return p

print('scb builders loaded')

# ---------------------------------------------------------------------------
# MPU (optional on both v6-M and v7-M)
# ---------------------------------------------------------------------------
def build_mpu_v6m():
    p, regs = mk_peripheral('MPU', '0xE000ED90',
        'Memory Protection Unit -- OPTIONAL on ARMv6-M: never present on plain '
        'Cortex-M0, but commonly implemented on Cortex-M0+ and SC000 '
        '(__MPU_PRESENT=1). No alias registers on this profile (verified against '
        'CMSIS core_cm0plus.h: MPU_TYPE_RALIASES=1, i.e. RBAR/RASR only).', size='0x14')
    regs.append(mkreg('TYPE', '0x0', 'read-only', '0x00000000', [
        mkfield('SEPARATE', 0, 1), mkfield('DREGION', 8, 8), mkfield('IREGION', 16, 8),
    ], desc='MPU Type Register'))
    regs.append(mkreg('CTRL', '0x4', 'read-write', '0x00000000', [
        mkfield('ENABLE', 0, 1), mkfield('HFNMIENA', 1, 1), mkfield('PRIVDEFENA', 2, 1),
    ], desc='MPU Control Register'))
    regs.append(mkreg('RNR', '0x8', 'read-write', '0x00000000', [mkfield('REGION', 0, 8)],
                       desc='MPU Region Number Register'))
    regs.append(mkreg('RBAR', '0xC', 'read-write', '0x00000000', [
        mkfield('REGION', 0, 4), mkfield('VALID', 4, 1), mkfield('ADDR', 8, 24),
    ], desc='MPU Region Base Address Register'))
    regs.append(mkreg('RASR', '0x10', 'read-write', '0x00000000', [
        mkfield('ENABLE', 0, 1), mkfield('SIZE', 1, 5), mkfield('SRD', 8, 8),
        mkfield('B', 16, 1), mkfield('C', 17, 1), mkfield('S', 18, 1), mkfield('TEX', 19, 3),
        mkfield('AP', 24, 3), mkfield('XN', 28, 1),
    ], desc='MPU Region Attribute and Size Register'))
    return p

def build_mpu_v7m():
    p, regs = mk_peripheral('MPU', '0xE000ED90',
        'Memory Protection Unit -- OPTIONAL on ARMv7-M (__MPU_PRESENT). When present, '
        'includes 3 alias RBAR/RASR pairs so up to 4 regions can be programmed without '
        're-writing RNR each time (verified against CMSIS core_cm3.h/core_cm4.h: '
        'MPU_TYPE_RALIASES=4).', size='0x30')
    regs.append(mkreg('TYPE', '0x0', 'read-only', '0x00000000', [
        mkfield('SEPARATE', 0, 1), mkfield('DREGION', 8, 8), mkfield('IREGION', 16, 8),
    ], desc='MPU Type Register'))
    regs.append(mkreg('CTRL', '0x4', 'read-write', '0x00000000', [
        mkfield('ENABLE', 0, 1), mkfield('HFNMIENA', 1, 1), mkfield('PRIVDEFENA', 2, 1),
    ], desc='MPU Control Register'))
    regs.append(mkreg('RNR', '0x8', 'read-write', '0x00000000', [mkfield('REGION', 0, 8)],
                       desc='MPU Region Number Register'))
    rbar_fields = lambda: [mkfield('REGION', 0, 4), mkfield('VALID', 4, 1), mkfield('ADDR', 5, 27)]
    rasr_fields = lambda: [mkfield('ENABLE', 0, 1), mkfield('SIZE', 1, 5), mkfield('SRD', 8, 8),
                            mkfield('B', 16, 1), mkfield('C', 17, 1), mkfield('S', 18, 1),
                            mkfield('TEX', 19, 3), mkfield('AP', 24, 3), mkfield('XN', 28, 1)]
    offsets = [('0xC', ''), ('0x14', '_A1'), ('0x1C', '_A2'), ('0x24', '_A3')]
    for off, suf in offsets:
        regs.append(mkreg(f'RBAR{suf}', off, 'read-write', '0x00000000', rbar_fields(),
                           desc=f'MPU Region Base Address Register{" alias" if suf else ""}'))
        rasr_off = hex(int(off, 16) + 4)
        regs.append(mkreg(f'RASR{suf}', rasr_off, 'read-write', '0x00000000', rasr_fields(),
                           desc=f'MPU Region Attribute and Size Register{" alias" if suf else ""}'))
    return p

print('mpu builders loaded')
