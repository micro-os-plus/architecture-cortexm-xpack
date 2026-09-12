import os
import xml.etree.ElementTree as ET
import importlib.util

BASE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('bg', f'{BASE}/build_generic.py')
bg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bg)

# Identical boilerplate license text on every ARM-authored CMSIS_5 SVD/header
# template (Device/ARM/SVD/ARMCM*.svd, ARMv8MBL.svd, ARMv8MML.svd all carry
# this verbatim) -- inlined here so regeneration needs no local CMSIS_5 clone.
LICENSE_TEXT = (
    '                                                   \n'
    '    ARM Limited (ARM) is supplying this software for use with Cortex-M\\n\n'
    '    processor based microcontroller, but can be equally used for other\\n\n'
    '    suitable  processor architectures. This file can be freely distributed.\\n\n'
    '    Modifications to this file shall be clearly marked.\\n\n'
    '    \\n\n'
    '    THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED\\n\n'
    '    OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF\\n\n'
    '    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.\\n\n'
    '    ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR\\n\n'
    '    CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.\n'
    '  '
)


def make_device(name, series, desc, cpu_name, revision, mpu, fpu, vtor, priobits,
                 vendor_systick='false', extra_cpu=None):
    root = ET.Element('device', attrib={
        'xmlns:xsi': 'http://www.w3.org/2001/XMLSchema-instance',
        'schemaVersion': '1.3',
        'xsi:noNamespaceSchemaLocation': 'CMSIS-SVD.xsd',
    })
    ET.SubElement(root, 'vendor').text = 'ARM Ltd.'
    ET.SubElement(root, 'vendorID').text = 'ARM'
    ET.SubElement(root, 'name').text = name
    ET.SubElement(root, 'series').text = series
    ET.SubElement(root, 'version').text = '1.0'
    ET.SubElement(root, 'description').text = desc
    ET.SubElement(root, 'licenseText').text = LICENSE_TEXT
    cpu = ET.SubElement(root, 'cpu')
    ET.SubElement(cpu, 'name').text = cpu_name
    ET.SubElement(cpu, 'revision').text = revision
    ET.SubElement(cpu, 'endian').text = 'little'
    ET.SubElement(cpu, 'mpuPresent').text = mpu
    ET.SubElement(cpu, 'fpuPresent').text = fpu
    ET.SubElement(cpu, 'vtorPresent').text = vtor
    ET.SubElement(cpu, 'nvicPrioBits').text = priobits
    ET.SubElement(cpu, 'vendorSystickConfig').text = vendor_systick
    for tag, text in (extra_cpu or []):
        ET.SubElement(cpu, tag).text = text
    ET.SubElement(root, 'addressUnitBits').text = '8'
    ET.SubElement(root, 'width').text = '32'
    ET.SubElement(root, 'size').text = '32'
    ET.SubElement(root, 'access').text = 'read-write'
    ET.SubElement(root, 'resetValue').text = '0x00000000'
    ET.SubElement(root, 'resetMask').text = '0xFFFFFFFF'
    return root


def write_device(root, filename):
    ET.indent(root, space='  ')
    with open(f'{BASE}/{filename}', 'w') as f:
        f.write('<?xml version="1.0" encoding="utf-8"?>\n')
        f.write(ET.tostring(root, encoding='unicode'))


# ============================== ARMv6-M ====================================
root_v6 = make_device(
    name='Generic_ARMv6M',
    series='ARMv6-M architecture (Cortex-M0 / Cortex-M0+ / Cortex-M1 / SC000)',
    desc=('Generic ARMv6-M system-peripheral template: NVIC, SCB, SysTick, CoreDebug, '
          'and an optional MPU. Built from the verified union of what CMSIS_5\'s '
          'core_cm0.h and core_cm0plus.h declare for the architecture, NOT from any '
          'single vendor SVD (none of ST/Nordic/NXP\'s published SVDs define these). '
          'IMPORTANT: __VTOR_PRESENT, __MPU_PRESENT and __NVIC_PRIO_BITS are '
          'implementation-defined per ARMv6-M -- see the VTOR and MPU peripheral '
          'descriptions and the <cpu> block below for what varies by device. Field '
          'widths for priority registers (NVIC IPRn, SCB SHPR2/SHPR3) are kept at '
          'their full architectural 8 bits rather than narrowed to a specific '
          'device\'s __NVIC_PRIO_BITS, since that value differs per implementation '
          '(commonly 2, but the architecture permits others).'),
    cpu_name='CM0', revision='r0p0', mpu='false', fpu='false', vtor='false', priobits='2')

peripherals = ET.SubElement(root_v6, 'peripherals')
peripherals.append(bg.build_nvic_v6m())
peripherals.append(bg.build_scb_v6m())
peripherals.append(bg.build_mpu_v6m())
peripherals.append(bg.build_systick())
peripherals.append(bg.build_coredebug(v6m_note=True))
write_device(root_v6, 'armv6m-system-peripherals.svd')

# ============================== ARMv7-M ====================================
root_v7 = make_device(
    name='Generic_ARMv7M',
    series='ARMv7-M architecture (Cortex-M3 / Cortex-M4 / Cortex-M7 / SC300)',
    desc=('Generic ARMv7-M system-peripheral template: NVIC (sized to the full '
          'architectural 240-interrupt maximum), SCB (with the full fault-handling '
          'register set), SysTick, CoreDebug, and an optional MPU. Built from the '
          'verified union of what CMSIS_5\'s core_cm3.h/core_cm4.h declare for the '
          'architecture (both are identical at this level -- the FPU-specific '
          'register block that exists on Cortex-M4/M7 is deliberately NOT included '
          'here, since plain Cortex-M3 has no FPU at all; ask if you want that added '
          'separately). Unlike ARMv6-M, the CoreDebug and SCB.SHCSR/DFSR registers ARE '
          'directly accessible from application code on ARMv7-M, not DAP-only.'),
    cpu_name='CM3', revision='r0p1', mpu='false', fpu='false', vtor='true', priobits='3')

peripherals7 = ET.SubElement(root_v7, 'peripherals')
peripherals7.append(bg.build_nvic_v7m())
peripherals7.append(bg.build_scb_v7m())
peripherals7.append(bg.build_mpu_v7m())
peripherals7.append(bg.build_systick())
peripherals7.append(bg.build_coredebug(v6m_note=False))
write_device(root_v7, 'armv7m-system-peripherals.svd')

# ========================== ARMv8-M Baseline ================================
root_v8bl = make_device(
    name='Generic_ARMv8MBaseline',
    series='ARMv8-M Baseline architecture (Cortex-M23)',
    desc=('Generic ARMv8-M Baseline system-peripheral template: NVIC, SCB, SysTick, '
          'DCB (renamed CoreDebug), and an optional MPU -- plus the optional Security '
          'Extension (TrustZone) SAU peripheral and the Secure/Non-secure fields on '
          'NVIC/SCB/DCB. Built from CMSIS_5\'s core_armv8mbl.h (Cortex-M23\'s core '
          'header), NOT from any single vendor SVD. IMPORTANT: __VTOR_PRESENT, '
          '__MPU_PRESENT, __SAUREGION_PRESENT, whether the Security Extension is '
          'implemented at all, and __NVIC_PRIO_BITS are all implementation-defined per '
          'ARMv8-M Baseline device -- see the individual peripheral descriptions and '
          'the <cpu> block below. Field widths for priority registers (NVIC IPRn, SCB '
          'SHPR2/SHPR3) are kept at their full architectural 8 bits rather than '
          'narrowed to a specific device\'s __NVIC_PRIO_BITS. Baseline keeps ARMv6-M\'s '
          'minimal fault model (no MemManage/BusFault/UsageFault/SecureFault/'
          'Debug-Monitor handlers, so no SHPR1/CFSR/HFSR/DFSR/NSACR/CPACR/SFSR/SFAR/'
          'ID_* registers either) but, unlike ARMv6-M, SHCSR is normally accessible '
          'from application code, not DAP-only.'),
    cpu_name='CM23', revision='r0p0', mpu='false', fpu='false', vtor='false', priobits='2')

peripherals_v8bl = ET.SubElement(root_v8bl, 'peripherals')
peripherals_v8bl.append(bg.build_nvic_v8m(mainline=False))
peripherals_v8bl.append(bg.build_scb_v8m_baseline())
peripherals_v8bl.append(bg.build_mpu_v8m(mainline=False))
peripherals_v8bl.append(bg.build_sau_v8m())
peripherals_v8bl.append(bg.build_systick())
peripherals_v8bl.append(bg.build_dcb_v8m(mainline=False))
write_device(root_v8bl, 'armv8m-baseline-system-peripherals.svd')

# ========================== ARMv8-M Mainline =================================
root_v8ml = make_device(
    name='Generic_ARMv8MMainline',
    series='ARMv8-M Mainline architecture (Cortex-M33 / Cortex-M35P / Cortex-M55 / Cortex-M85)',
    desc=('Generic ARMv8-M Mainline system-peripheral template: NVIC (sized to the '
          'full architectural 496-interrupt maximum), SCB (the full ARMv7-M '
          'fault-handling set plus the Security Extension additions), SysTick, DCB '
          '(renamed CoreDebug), and an optional MPU with RBAR/RLAR base+limit regions '
          '-- plus the optional Security Extension (TrustZone) SAU peripheral. Built '
          'from CMSIS_5\'s core_armv8mml.h (Cortex-M33\'s core header), NOT from any '
          'single vendor SVD. The Security Extension itself, SAU region count, MPU '
          'presence, and FPU/DSP presence are all implementation-defined per device -- '
          'fields like NVIC.ITNS, SCB.NSACR/SFSR/SFAR, and the STTNS/SYSRESETREQS/'
          'BFHFNMINS/PRIS/SLEEPDEEPS/SECUREFAULT* bits are Secure-only and RAZ/WI (or '
          'fixed) when the Security Extension is absent. Deliberately excluded, same '
          'rationale as the ARMv7-M file: the FPU/MVFR/cache-maintenance register '
          'blocks (CLIDR/CTR/CCSIDR/CSSELR, ICIALLU..BPIALL, CCR.DC/IC/BP -- '
          'Cortex-M55/M85-specific cache support) and trace components (ITM/DWT/TPIU).'),
    cpu_name='CM33', revision='r0p0', mpu='false', fpu='false', vtor='true', priobits='3')

peripherals_v8ml = ET.SubElement(root_v8ml, 'peripherals')
peripherals_v8ml.append(bg.build_nvic_v8m(mainline=True))
peripherals_v8ml.append(bg.build_scb_v8m_mainline())
peripherals_v8ml.append(bg.build_mpu_v8m(mainline=True))
peripherals_v8ml.append(bg.build_sau_v8m())
peripherals_v8ml.append(bg.build_systick())
peripherals_v8ml.append(bg.build_dcb_v8m(mainline=True))
write_device(root_v8ml, 'armv8m-mainline-system-peripherals.svd')

print('wrote all four files')

# ---- sanity pass ----
for fname in ('armv6m-system-peripherals.svd', 'armv7m-system-peripherals.svd',
              'armv8m-baseline-system-peripherals.svd', 'armv8m-mainline-system-peripherals.svd'):
    path = f'{BASE}/{fname}'
    chk = ET.parse(path)
    per = chk.getroot().find('peripherals')
    print('\n===', path, '===')
    for p in per.findall('peripheral'):
        base = int(p.find('baseAddress').text, 16)
        names = []
        for r in p.find('registers').findall('register'):
            nm = r.find('name').text
            dim = r.find('dim')
            names.append(f'{nm}[{dim.text}]' if dim is not None else nm)
        print(' ', p.find('name').text, hex(base), '->', names)
