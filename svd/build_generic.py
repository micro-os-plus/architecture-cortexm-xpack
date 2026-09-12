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

def mk_peripheral(name, base_addr, desc, group=None, size='0x400', address_blocks=None):
    # address_blocks, when given, overrides `size` with a list of (offset, size)
    # pairs -- needed where a peripheral's own registers are not contiguous
    # because another peripheral's address range is spliced in between them
    # (e.g. ARMv8-M's SCB, which has MPU and SAU living inside its own
    # 0xE000ED00-0xE000EDEF page).
    p = ET.Element('peripheral')
    ET.SubElement(p, 'name').text = name
    ET.SubElement(p, 'description').text = desc
    ET.SubElement(p, 'groupName').text = group or name
    ET.SubElement(p, 'baseAddress').text = base_addr
    for off, sz in (address_blocks or [('0x0', size)]):
        ab = ET.SubElement(p, 'addressBlock')
        ET.SubElement(ab, 'offset').text = off
        ET.SubElement(ab, 'size').text = sz
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

# ---------------------------------------------------------------------------
# ARMv8-M (Baseline: Cortex-M23; Mainline: Cortex-M33/M35P/M55/M85).
# Verified against CMSIS_5 core_armv8mbl.h / core_armv8mml.h. Both profiles
# add the optional Security Extension (TrustZone): SAU, NVIC.ITNS, SCB.NSACR/
# SFSR/SFAR, and Secure-only fields scattered across ICSR/AIRCR/SCR/SHCSR/DCB
# -- all RAZ/WI or fixed-value when the Security Extension is not implemented.
# ---------------------------------------------------------------------------

def build_nvic_v8m(mainline):
    desc = ('Nested Vectored Interrupt Controller. ARMv8-M packs interrupt state into '
            '16 banks of 32 bits each (ISER/ICER/ISPR/ICPR/IABR/ITNS) plus 124 32-bit '
            'IPR words, covering the full architectural maximum of 496 external '
            'interrupts (verified against CMSIS core_armv8mbl.h/core_armv8mml.h: IPR is '
            'a 496-byte array, i.e. 124 words of 4 packed 8-bit priorities each -- same '
            'total on both profiles). ITNS (Interrupt Target Non-Secure) is new versus '
            'ARMv6-M/ARMv7-M: one bit per interrupt selecting whether it targets the '
            'Secure or the Non-secure state; Secure-only access, and RAZ/WI when the '
            'Security Extension is not implemented. ' +
            ('Mainline additionally has STIR, at the same offset as on ARMv7-M.'
             if mainline else
             'No STIR on Baseline, matching ARMv6-M (STIR is Mainline-only across the '
             'whole Cortex-M family).'))
    size = '0xE04' if mainline else '0x4F0'
    p, regs = mk_peripheral('NVIC', '0xE000E100', desc, size=size)
    banks = [(0x000, 'ISER', 'Interrupt Set-Enable Register', 'read-write'),
             (0x080, 'ICER', 'Interrupt Clear-Enable Register', 'read-write'),
             (0x100, 'ISPR', 'Interrupt Set-Pending Register', 'read-write'),
             (0x180, 'ICPR', 'Interrupt Clear-Pending Register', 'read-write'),
             (0x200, 'IABR', 'Interrupt Active Bit Register', 'read-only'),
             (0x280, 'ITNS', 'Interrupt Target Non-Secure Register -- Security '
                             'Extension only, Secure-accessible', 'read-write')]
    for off, nm, d, acc in banks:
        regs.append(mkregarray(nm, hex(off), 16, '0x4', acc, '0x00000000',
                                fields=[mkfield('BITS', 0, 32)], desc=d))
    regs.append(mkregarray('IPR', '0x300', 124, '0x4', 'read-write', '0x00000000',
                            fields=priofields(),
                            desc='Interrupt Priority Registers 0-123 (cover IRQ0..IRQ495)'))
    if mainline:
        regs.append(mkreg('STIR', '0xE00', 'write-only', '0x00000000', [
            mkfield('INTID', 0, 9, 'Interrupt ID of the interrupt to trigger, 0-495'),
        ], desc='Software Trigger Interrupt Register. Only usable if SCB->CCR.USERSETMPEND '
                'allows unprivileged access and the device implements this register.'))
    return p

print('nvic v8m builder loaded')

# ---------------------------------------------------------------------------
# SCB -- shared field-list helpers (ICSR/AIRCR/SCR/CCR are identical in shape
# on both ARMv8-M profiles; only whether the register/field exists at all
# differs, handled by the two build_scb_v8m_* functions below).
# ---------------------------------------------------------------------------
def icsr_v8m_fields():
    return [
        mkfield('VECTACTIVE', 0, 9),
        mkfield('RETTOBASE', 11, 1, access='read-only'),
        mkfield('VECTPENDING', 12, 9),
        mkfield('ISRPENDING', 22, 1, access='read-only'),
        mkfield('ISRPREEMPT', 23, 1, access='read-only'),
        mkfield('STTNS', 24, 1, 'Secure Transition To Non-secure -- Security Extension '
                'only, banked, RAZ/WI from Non-secure state'),
        mkfield('PENDSTCLR', 25, 1, access='write-only'),
        mkfield('PENDSTSET', 26, 1),
        mkfield('PENDSVCLR', 27, 1, access='write-only'),
        mkfield('PENDSVSET', 28, 1),
        mkfield('PENDNMICLR', 30, 1, 'New in ARMv8-M', access='write-only'),
        mkfield('PENDNMISET', 31, 1, 'aka NMIPENDSET'),
    ]

def aircr_v8m_fields(mainline):
    f = [
        mkfield('VECTCLRACTIVE', 1, 1, 'Reserved for debug use'),
        mkfield('SYSRESETREQ', 2, 1),
        mkfield('SYSRESETREQS', 3, 1, 'SYSRESETREQ traps to Secure state when set -- '
                'Security Extension only'),
    ]
    if mainline:
        f.append(mkfield('PRIGROUP', 8, 3, 'Interrupt priority grouping'))
    f += [
        mkfield('BFHFNMINS', 13, 1, 'BusFault/HardFault/NMI target Non-secure state '
                'when set -- Security Extension only'),
        mkfield('PRIS', 14, 1, 'Prioritize Secure exceptions -- Security Extension only'),
        mkfield('ENDIANESS', 15, 1, access='read-only'),
        mkfield('VECTKEY', 16, 16, 'Write 0x05FA to permit the write; reads as 0xFA05'),
    ]
    return f

def scr_v8m_fields():
    return [
        mkfield('SLEEPONEXIT', 1, 1),
        mkfield('SLEEPDEEP', 2, 1),
        mkfield('SLEEPDEEPS', 3, 1, 'SLEEPDEEP usable from Non-secure state -- Security '
                'Extension only'),
        mkfield('SEVONPEND', 4, 1),
    ]

def ccr_v8m_fields():
    # No NONBASETHRDENA / STKALIGN: ARMv8-M makes both behaviors unconditional
    # (thread mode always enterable at any priority; stack always 8-byte
    # aligned on exception entry), so neither bit exists any more. Also
    # excludes CCR.DC/IC/BP (cache enable/branch-predictor-invalidate), which
    # only exist on cache-capable implementations (Cortex-M55/M85) -- out of
    # scope, same rationale as the FPU/cache exclusions on the ARMv7-M file.
    return [
        mkfield('USERSETMPEND', 1, 1),
        mkfield('UNALIGN_TRP', 3, 1),
        mkfield('DIV_0_TRP', 4, 1),
        mkfield('BFHFNMIGN', 8, 1),
        mkfield('STKOFHFNMIGN', 10, 1, 'Ignore stack overflow while handling a HardFault/'
                'NMI/FAULTMASK-escalated fault -- new in ARMv8-M'),
    ]

def build_scb_v8m_baseline():
    p, regs = mk_peripheral('SCB', '0xE000ED00',
        'System Control Block, ARMv8-M Baseline register set. Verified against CMSIS_5 '
        'core_armv8mbl.h. Minimal fault model like ARMv6-M -- no SHPR1, no CFSR/HFSR/'
        'MMFAR/BFAR/AFSR/CPACR/NSACR/SFSR/SFAR/ID_* registers at all (no MemManage/'
        'BusFault/UsageFault/SecureFault handlers on Baseline), no AIRCR.PRIGROUP '
        '(fixed priority split, like ARMv6-M). Unlike ARMv6-M, ICSR.RETTOBASE exists and '
        'SHCSR is normally accessible from application code, not DAP-only. Gains the '
        'Security Extension fields (ICSR.STTNS, AIRCR.SYSRESETREQS/BFHFNMINS/PRIS, '
        'SCR.SLEEPDEEPS, SHCSR.HARDFAULT*/NMIACT) -- Secure-only/RAZ-WI when the '
        'Security Extension is not implemented. CCR.STKALIGN and CCR.NONBASETHRDENA no '
        'longer exist as configurable bits, see the CCR register below.', size='0x28')
    regs.append(mkreg('CPUID', '0x0', 'read-only', '0x00000000', cpuid_fields(),
                       desc='CPUID Base Register. Reset value is implementation/revision specific.'))
    regs.append(mkreg('ICSR', '0x4', 'read-write', '0x00000000', icsr_v8m_fields(),
                       desc='Interrupt Control and State Register'))
    regs.append(mkreg('VTOR', '0x8', 'read-write', '0x00000000', [
        mkfield('TBLOFF', 7, 25),
    ], desc='Vector Table Offset Register -- OPTIONAL: implementation-defined whether '
            'present (__VTOR_PRESENT); reads as 0 / reserved when absent.'))
    regs.append(mkreg('AIRCR', '0xC', 'read-write', '0x00000000', aircr_v8m_fields(False),
                       desc='Application Interrupt and Reset Control Register. No '
                            'PRIGROUP (fixed priority grouping, like ARMv6-M).'))
    regs.append(mkreg('SCR', '0x10', 'read-write', '0x00000000', scr_v8m_fields(),
                       desc='System Control Register'))
    regs.append(mkreg('CCR', '0x14', 'read-write', '0x00000000', ccr_v8m_fields(),
                       desc='Configuration and Control Register'))
    regs.append(mkreg('SHPR2', '0x1C', 'read-write', '0x00000000', [
        mkfield('PRI_11', 24, 8, 'Priority of exception 11, SVCall'),
    ], desc='System Handler Priority Register 2'))
    regs.append(mkreg('SHPR3', '0x20', 'read-write', '0x00000000', [
        mkfield('PRI_14', 16, 8, 'Priority of exception 14, PendSV'),
        mkfield('PRI_15', 24, 8, 'Priority of exception 15, SysTick'),
    ], desc='System Handler Priority Register 3'))
    regs.append(mkreg('SHCSR', '0x24', 'read-write', '0x00000000', [
        mkfield('HARDFAULTACT', 2, 1),
        mkfield('NMIACT', 5, 1, 'New in ARMv8-M: HardFault/NMI gets a trackable active bit'),
        mkfield('SVCALLACT', 7, 1),
        mkfield('PENDSVACT', 10, 1),
        mkfield('SYSTICKACT', 11, 1),
        mkfield('SVCALLPENDED', 15, 1),
        mkfield('HARDFAULTPENDED', 21, 1, 'New in ARMv8-M'),
    ], desc='System Handler Control and State Register'))
    return p

def build_scb_v8m_mainline():
    p, regs = mk_peripheral('SCB', '0xE000ED00',
        'System Control Block, ARMv8-M Mainline register set. Verified against CMSIS_5 '
        'core_armv8mml.h. Superset of ARMv7-M\'s SCB plus the Security Extension: NSACR '
        '(Non-secure Access Control), SFSR/SFAR (Secure Fault Status/Address -- '
        'physically the same registers CMSIS also exposes via SAU->SFSR/SFAR; modeled '
        'once here, under SCB, since SAU and SCB share the same 0xE000ED00-0xE000EDEF '
        'page -- see the two <addressBlock> entries below and the SAU peripheral\'s '
        'description), and extra ICSR/AIRCR/SCR/SHCSR/CFSR bits (see the individual '
        'register descriptions). ID_ISAR now has 6 words (ID_ISAR5 added) versus 5 on '
        'ARMv7-M; the ID_* register names themselves also changed (ADR/DFR -> ID_AFR/'
        'ID_DFR) to match current CMSIS naming for ARMv8-M. Deliberately excluded, same '
        'rationale as ARMv7-M: CFSR.MLSPERR/LSPERR (FPU lazy-stacking bits) are '
        'documented but the FPU block itself is not modeled, and the cache-maintenance '
        'registers present on cache-capable implementations (CLIDR/CTR/CCSIDR/CSSELR, '
        'ICIALLU..BPIALL, MVFR0-2, CCR.DC/IC/BP -- Cortex-M55/M85-specific) are out of '
        'scope, same as trace components (ITM/DWT/TPIU).',
        address_blocks=[('0x0', '0x90'), ('0xE4', '0x8')])
    regs.append(mkreg('CPUID', '0x0', 'read-only', '0x00000000', cpuid_fields(),
                       desc='CPUID Base Register. Reset value is implementation/revision specific.'))
    regs.append(mkreg('ICSR', '0x4', 'read-write', '0x00000000', icsr_v8m_fields(),
                       desc='Interrupt Control and State Register'))
    regs.append(mkreg('VTOR', '0x8', 'read-write', '0x00000000', [
        mkfield('TBLOFF', 7, 25),
    ], desc='Vector Table Offset Register'))
    regs.append(mkreg('AIRCR', '0xC', 'read-write', '0x00000000', aircr_v8m_fields(True),
                       desc='Application Interrupt and Reset Control Register'))
    regs.append(mkreg('SCR', '0x10', 'read-write', '0x00000000', scr_v8m_fields(),
                       desc='System Control Register'))
    regs.append(mkreg('CCR', '0x14', 'read-write', '0x00000000', ccr_v8m_fields(),
                       desc='Configuration and Control Register'))
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
        mkfield('MEMFAULTACT', 0, 1), mkfield('BUSFAULTACT', 1, 1),
        mkfield('HARDFAULTACT', 2, 1, 'New in ARMv8-M'),
        mkfield('USGFAULTACT', 3, 1),
        mkfield('SECUREFAULTACT', 4, 1, 'Security Extension only'),
        mkfield('NMIACT', 5, 1, 'New in ARMv8-M'),
        mkfield('SVCALLACT', 7, 1), mkfield('MONITORACT', 8, 1), mkfield('PENDSVACT', 10, 1),
        mkfield('SYSTICKACT', 11, 1), mkfield('USGFAULTPENDED', 12, 1),
        mkfield('MEMFAULTPENDED', 13, 1), mkfield('BUSFAULTPENDED', 14, 1),
        mkfield('SVCALLPENDED', 15, 1), mkfield('MEMFAULTENA', 16, 1),
        mkfield('BUSFAULTENA', 17, 1), mkfield('USGFAULTENA', 18, 1),
        mkfield('SECUREFAULTENA', 19, 1, 'Security Extension only'),
        mkfield('SECUREFAULTPENDED', 20, 1, 'Security Extension only'),
        mkfield('HARDFAULTPENDED', 21, 1, 'New in ARMv8-M'),
    ], desc='System Handler Control and State Register'))
    regs.append(mkreg('CFSR', '0x28', 'read-write', '0x00000000', [
        mkfield('IACCVIOL', 0, 1, 'MMFSR'), mkfield('DACCVIOL', 1, 1, 'MMFSR'),
        mkfield('MUNSTKERR', 3, 1, 'MMFSR'), mkfield('MSTKERR', 4, 1, 'MMFSR'),
        mkfield('MLSPERR', 5, 1, 'MMFSR -- lazy FP state preservation error, only '
                'meaningful with an FPU'),
        mkfield('MMARVALID', 7, 1, 'MMFSR', access='read-only'),
        mkfield('IBUSERR', 8, 1, 'BFSR'), mkfield('PRECISERR', 9, 1, 'BFSR'),
        mkfield('IMPRECISERR', 10, 1, 'BFSR'), mkfield('UNSTKERR', 11, 1, 'BFSR'),
        mkfield('STKERR', 12, 1, 'BFSR'),
        mkfield('LSPERR', 13, 1, 'BFSR -- lazy FP state preservation error, only '
                'meaningful with an FPU'),
        mkfield('BFARVALID', 15, 1, 'BFSR', access='read-only'),
        mkfield('UNDEFINSTR', 16, 1, 'UFSR'), mkfield('INVSTATE', 17, 1, 'UFSR'),
        mkfield('INVPC', 18, 1, 'UFSR'), mkfield('NOCP', 19, 1, 'UFSR'),
        mkfield('STKOF', 20, 1, 'UFSR -- Stack Overflow, new in ARMv8-M'),
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
    regs.append(mkregarray('ID_PFR', '0x40', 2, '0x4', 'read-only', '0x00000000',
                            desc='Processor Feature Register, implementation defined'))
    regs.append(mkreg('ID_DFR', '0x48', 'read-only', '0x00000000', [],
                       desc='Debug Feature Register, implementation defined'))
    regs.append(mkreg('ID_AFR', '0x4C', 'read-only', '0x00000000', [],
                       desc='Auxiliary Feature Register, implementation defined'))
    regs.append(mkregarray('ID_MMFR', '0x50', 4, '0x4', 'read-only', '0x00000000',
                            desc='Memory Model Feature Register, implementation defined'))
    regs.append(mkregarray('ID_ISAR', '0x60', 6, '0x4', 'read-only', '0x00000000',
                            desc='Instruction Set Attributes Register, implementation defined '
                                 '-- 6 words on ARMv8-M (ID_ISAR5 added), vs. 5 on ARMv7-M'))
    regs.append(mkreg('CPACR', '0x88', 'read-write', '0x00000000', [
        mkfield('CP10', 20, 2, 'FPU access privileges -- only meaningful if an FPU is implemented'),
        mkfield('CP11', 22, 2, 'FPU access privileges -- only meaningful if an FPU is implemented'),
    ], desc='Coprocessor Access Control Register'))
    regs.append(mkreg('NSACR', '0x8C', 'read-write', '0x00000000', [
        mkfield('CP', 0, 8, 'Non-secure access permission for coprocessors CP0-CP7, one '
                'bit each -- Security Extension only'),
        mkfield('CP10', 10, 1, 'Non-secure access permission for the FPU (CP10) -- '
                'Security Extension only, meaningful only if an FPU is implemented'),
        mkfield('CP11', 11, 1, 'Non-secure access permission for the FPU (CP11) -- '
                'Security Extension only, meaningful only if an FPU is implemented'),
    ], desc='Non-Secure Access Control Register -- Security Extension only'))
    regs.append(mkreg('SFSR', '0xE4', 'read-write', '0x00000000', [
        mkfield('INVEP', 0, 1), mkfield('INVIS', 1, 1), mkfield('INVER', 2, 1),
        mkfield('AUVIOL', 3, 1), mkfield('INVTRAN', 4, 1),
        mkfield('LSPERR', 5, 1, 'Only meaningful with an FPU'),
        mkfield('SFARVALID', 6, 1, access='read-only'), mkfield('LSERR', 7, 1),
    ], desc='Secure Fault Status Register -- Security Extension only. Physically the same '
            'register as SAU->SFSR (CMSIS exposes it under both SCB_Type and SAU_Type).'))
    regs.append(mkreg('SFAR', '0xE8', 'read-write', '0x00000000', [mkfield('ADDRESS', 0, 32)],
        desc='Secure Fault Address Register, valid only when SFSR.SFARVALID is set -- '
             'Security Extension only. Physically the same register as SAU->SFAR.'))
    return p

print('scb v8m builders loaded')

# ---------------------------------------------------------------------------
# MPU -- optional on both ARMv8-M profiles. Region format changed from
# ARMv7-M's base+size-encoded RBAR/RASR to base+limit RBAR/RLAR, and gained
# MAIR0/MAIR1 memory-attribute indirection registers.
# ---------------------------------------------------------------------------
def build_mpu_v8m(mainline):
    desc = ('Memory Protection Unit -- OPTIONAL on ARMv8-M ' +
            ('Mainline' if mainline else 'Baseline') + ' (__MPU_PRESENT). Region format '
            'changed from ARMv7-M\'s RBAR/RASR (base + size-encoded) to RBAR/RLAR '
            '(base + limit), and gained MAIR0/MAIR1 memory-attribute indirection '
            'registers (verified against CMSIS core_armv8mbl.h/core_armv8mml.h). ' +
            ('Includes 3 alias RBAR/RLAR pairs, same as ARMv7-M (MPU_TYPE_RALIASES=4).'
             if mainline else
             'No alias registers on Baseline, matching ARMv6-M\'s MPU shape.'))
    p, regs = mk_peripheral('MPU', '0xE000ED90', desc, size='0x38')
    regs.append(mkreg('TYPE', '0x0', 'read-only', '0x00000000', [
        mkfield('SEPARATE', 0, 1), mkfield('DREGION', 8, 8), mkfield('IREGION', 16, 8),
    ], desc='MPU Type Register'))
    regs.append(mkreg('CTRL', '0x4', 'read-write', '0x00000000', [
        mkfield('ENABLE', 0, 1), mkfield('HFNMIENA', 1, 1), mkfield('PRIVDEFENA', 2, 1),
    ], desc='MPU Control Register'))
    regs.append(mkreg('RNR', '0x8', 'read-write', '0x00000000', [mkfield('REGION', 0, 8)],
                       desc='MPU Region Number Register'))
    rbar_fields = lambda: [mkfield('XN', 0, 1, 'Execute-never'),
                            mkfield('AP', 1, 2, 'Access permissions'),
                            mkfield('SH', 3, 2, 'Shareability'),
                            mkfield('BASE', 5, 27, 'Base address, 32-byte aligned')]
    rlar_fields = lambda: [mkfield('EN', 0, 1, 'Region enable'),
                            mkfield('AttrIndx', 1, 3, 'MAIR index'),
                            mkfield('LIMIT', 5, 27, 'Limit address, 32-byte aligned')]
    offsets = [('0xC', '')] + ([('0x14', '_A1'), ('0x1C', '_A2'), ('0x24', '_A3')]
                                if mainline else [])
    for off, suf in offsets:
        regs.append(mkreg(f'RBAR{suf}', off, 'read-write', '0x00000000', rbar_fields(),
                           desc=f'MPU Region Base Address Register{" alias" if suf else ""}'))
        rlar_off = hex(int(off, 16) + 4)
        regs.append(mkreg(f'RLAR{suf}', rlar_off, 'read-write', '0x00000000', rlar_fields(),
                           desc=f'MPU Region Limit Address Register{" alias" if suf else ""}'))
    regs.append(mkreg('MAIR0', '0x30', 'read-write', '0x00000000', [
        mkfield('Attr0', 0, 8), mkfield('Attr1', 8, 8), mkfield('Attr2', 16, 8), mkfield('Attr3', 24, 8),
    ], desc='MPU Memory Attribute Indirection Register 0'))
    regs.append(mkreg('MAIR1', '0x34', 'read-write', '0x00000000', [
        mkfield('Attr4', 0, 8), mkfield('Attr5', 8, 8), mkfield('Attr6', 16, 8), mkfield('Attr7', 24, 8),
    ], desc='MPU Memory Attribute Indirection Register 1'))
    return p

print('mpu v8m builder loaded')

# ---------------------------------------------------------------------------
# SAU -- the Security Attribution Unit, new in ARMv8-M. Present only when the
# Security Extension is implemented; identical register layout on both
# profiles (SFSR/SFAR, which CMSIS also exposes via SAU_Type, are modeled
# once under SCB instead -- see build_scb_v8m_mainline).
# ---------------------------------------------------------------------------
def build_sau_v8m():
    p, regs = mk_peripheral('SAU', '0xE000EDD0',
        'Security Attribution Unit -- present only when the Security Extension '
        '(TrustZone) is implemented; absent entirely on a Non-secure-only/single-state '
        'part. RNR/RBAR/RLAR are further optional within that: only present when '
        '__SAUREGION_PRESENT=1, i.e. the implementation has at least one programmable '
        'SAU region (a part can implement the Security Extension with the whole memory '
        'map fixed Secure/Non-secure by IDAU alone and no SAU regions at all). Verified '
        'against CMSIS core_armv8mbl.h/core_armv8mml.h (identical on both profiles). '
        'SFSR/SFAR are modeled once, under SCB, since SAU_Type and SCB_Type both expose '
        'the same physical registers at the same address on Mainline; Baseline has '
        'neither (no SecureFault handler exists there).', size='0x14')
    regs.append(mkreg('CTRL', '0x0', 'read-write', '0x00000000', [
        mkfield('ENABLE', 0, 1),
        mkfield('ALLNS', 1, 1, 'All Non-secure -- forces the whole memory map '
                'Non-secure when SAU is disabled'),
    ], desc='SAU Control Register'))
    regs.append(mkreg('TYPE', '0x4', 'read-only', '0x00000000', [
        mkfield('SREGION', 0, 8, 'Number of implemented SAU regions'),
    ], desc='SAU Type Register'))
    regs.append(mkreg('RNR', '0x8', 'read-write', '0x00000000', [mkfield('REGION', 0, 8)],
        desc='SAU Region Number Register -- only present if __SAUREGION_PRESENT'))
    regs.append(mkreg('RBAR', '0xC', 'read-write', '0x00000000', [
        mkfield('BADDR', 5, 27, 'Base address, 32-byte aligned'),
    ], desc='SAU Region Base Address Register -- only present if __SAUREGION_PRESENT'))
    regs.append(mkreg('RLAR', '0x10', 'read-write', '0x00000000', [
        mkfield('ENABLE', 0, 1), mkfield('NSC', 1, 1, 'Non-secure Callable'),
        mkfield('LADDR', 5, 27, 'Limit address, 32-byte aligned'),
    ], desc='SAU Region Limit Address Register -- only present if __SAUREGION_PRESENT'))
    return p

print('sau v8m builder loaded')

# ---------------------------------------------------------------------------
# DCB -- CMSIS's ARMv8-M name for what core_cm0.h/core_cm3.h call CoreDebug
# (same base address, 0xE000EDF0). Adds DAUTHCTRL and DSCSR; DHCSR/DEMCR gain
# Security Extension bits and, on Mainline, keep the full ARMv7-M fault/
# monitor bit set.
# ---------------------------------------------------------------------------
def build_dcb_v8m(mainline):
    desc = ('Debug Control Block -- CMSIS\'s ARMv8-M name for what core_cm0.h/core_cm3.h '
            'call CoreDebug (same base address, 0xE000EDF0). Adds DAUTHCTRL (Debug '
            'Authentication Control) and DSCSR (Debug Security Control and Status) '
            'versus ARMv6-M/ARMv7-M\'s CoreDebug -- both Security-Extension-related, '
            'present regardless of profile. Verified against CMSIS core_armv8mbl.h/'
            'core_armv8mml.h.' +
            ('' if mainline else ' Baseline\'s DHCSR has no C_SNAPSTALL bit, and DEMCR '
             'only defines TRCENA/VC_CORERESET/VC_HARDERR -- no other VC_*, no MON_*, no '
             'MONPRKEY/UMON_EN/SDME/VC_SFERR -- matching Baseline\'s minimal exception '
             'model.'))
    p, regs = mk_peripheral('DCB', '0xE000EDF0', desc, size='0x1C')
    dhcsr_fields = [
        mkfield('C_DEBUGEN', 0, 1), mkfield('C_HALT', 1, 1), mkfield('C_STEP', 2, 1),
        mkfield('C_MASKINTS', 3, 1),
    ]
    if mainline:
        dhcsr_fields.append(mkfield('C_SNAPSTALL', 5, 1))
    dhcsr_fields += [
        mkfield('S_REGRDY', 16, 1, access='read-only'),
        mkfield('S_HALT', 17, 1, access='read-only'),
        mkfield('S_SLEEP', 18, 1, access='read-only'),
        mkfield('S_LOCKUP', 19, 1, access='read-only'),
        mkfield('S_SDE', 20, 1, 'Secure Debug Enabled -- Security Extension only', access='read-only'),
        mkfield('S_RETIRE_ST', 24, 1, access='read-only'),
        mkfield('S_RESET_ST', 25, 1, access='read-only'),
        mkfield('S_RESTART_ST', 26, 1, 'New in ARMv8-M', access='read-only'),
        mkfield('DBGKEY', 16, 16, 'Write 0xA05F to enable write access to bits [15:0]; reads as 0',
                access='write-only'),
    ]
    regs.append(mkreg('DHCSR', '0x0', 'read-write', '0x00000000', dhcsr_fields,
                       desc='Debug Halting Control and Status Register'))
    regs.append(mkreg('DCRSR', '0x4', 'write-only', '0x00000000', [
        mkfield('REGSEL', 0, 7), mkfield('REGWnR', 16, 1),
    ], desc='Debug Core Register Selector Register'))
    regs.append(mkreg('DCRDR', '0x8', 'read-write', '0x00000000', [
        mkfield('DBGTMP', 0, 32),
    ], desc='Debug Core Register Data Register'))
    if mainline:
        demcr_fields = [
            mkfield('VC_CORERESET', 0, 1, 'Reset Vector Catch'),
            mkfield('VC_MMERR', 4, 1, 'Debug trap on MemManage exception'),
            mkfield('VC_NOCPERR', 5, 1, 'Debug trap on UsageFault caused by no coprocessor'),
            mkfield('VC_CHKERR', 6, 1, 'Debug trap on UsageFault caused by checking error'),
            mkfield('VC_STATERR', 7, 1, 'Debug trap on UsageFault state error'),
            mkfield('VC_BUSERR', 8, 1, 'Debug trap on BusFault exception'),
            mkfield('VC_INTERR', 9, 1, 'Debug trap on interrupt/exception service errors'),
            mkfield('VC_HARDERR', 10, 1, 'Debug trap on HardFault exception'),
            mkfield('VC_SFERR', 11, 1, 'Debug trap on SecureFault exception -- Security '
                    'Extension only'),
            mkfield('MON_EN', 16, 1, 'Debug Monitor enable'),
            mkfield('MON_PEND', 17, 1, 'Debug Monitor pend'),
            mkfield('MON_STEP', 18, 1, 'Debug Monitor step'),
            mkfield('MON_REQ', 19, 1, 'Debug Monitor request'),
            mkfield('SDME', 20, 1, 'Secure DebugMonitor Enable -- Security Extension only'),
            mkfield('UMON_EN', 21, 1, 'Unprivileged Monitor Enable'),
            mkfield('MONPRKEY', 23, 1, 'Monitor Pend Request Key'),
            mkfield('TRCENA', 24, 1, 'Trace enable (DWT/ITM/ETM/TPIU) -- only meaningful '
                    'where those blocks exist'),
        ]
    else:
        demcr_fields = [
            mkfield('VC_CORERESET', 0, 1, 'Reset Vector Catch'),
            mkfield('VC_HARDERR', 10, 1, 'Debug trap on HardFault exception'),
            mkfield('TRCENA', 24, 1, 'Trace enable (DWT/ITM/ETM/TPIU) -- only meaningful '
                    'where those blocks exist'),
        ]
    regs.append(mkreg('DEMCR', '0xC', 'read-write', '0x00000000', demcr_fields,
                       desc='Debug Exception and Monitor Control Register'))
    regs.append(mkreg('DAUTHCTRL', '0x14', 'read-write', '0x00000000', [
        mkfield('SPIDENSEL', 0, 1, 'Secure invasive debug enable select'),
        mkfield('INTSPIDEN', 1, 1, 'Internal Secure invasive debug enable'),
        mkfield('SPNIDENSEL', 2, 1, 'Secure non-invasive debug enable select'),
        mkfield('INTSPNIDEN', 3, 1, 'Internal Secure non-invasive debug enable'),
    ], desc='Debug Authentication Control Register -- Security Extension only'))
    regs.append(mkreg('DSCSR', '0x18', 'read-write', '0x00000000', [
        mkfield('SBRSELEN', 0, 1, 'Secure banked register select enable'),
        mkfield('SBRSEL', 1, 1, 'Secure banked register select'),
        mkfield('CDS', 16, 1, 'Current Domain Secure', access='read-only'),
        mkfield('CDSKEY', 17, 1, 'CDS write-enable key', access='write-only'),
    ], desc='Debug Security Control and Status Register -- Security Extension only'))
    return p

print('dcb v8m builder loaded')
