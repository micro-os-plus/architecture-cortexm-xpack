import xml.etree.ElementTree as ET
import importlib.util

BASE = '/tmp/claude-0/-home-claude/66138eca-62cf-5e0a-959c-5a69e5b4eed7/scratchpad/svd'
spec = importlib.util.spec_from_file_location('bg', f'{BASE}/build_generic.py')
bg = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bg)

def make_device(name, series, desc, cpu_name, revision, mpu, fpu, vtor, priobits, license_src):
    t = ET.parse(license_src)
    root = t.getroot()
    root.find('name').text = name
    root.find('series').text = series
    root.find('description').text = desc
    cpu = root.find('cpu')
    cpu.find('name').text = cpu_name
    cpu.find('revision').text = revision
    cpu.find('mpuPresent').text = mpu
    cpu.find('fpuPresent').text = fpu
    cpu.find('vtorPresent').text = vtor
    cpu.find('nvicPrioBits').text = priobits
    return root

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
    cpu_name='CM0', revision='r0p0', mpu='false', fpu='false', vtor='false', priobits='2',
    license_src=f'{BASE}/cmsis5/repo/Device/ARM/SVD/ARMCM0.svd')

peripherals = ET.SubElement(root_v6, 'peripherals')
peripherals.append(bg.build_nvic_v6m())
peripherals.append(bg.build_scb_v6m())
peripherals.append(bg.build_mpu_v6m())
peripherals.append(bg.build_systick())
peripherals.append(bg.build_coredebug(v6m_note=True))

ET.indent(root_v6, space='  ')
with open(f'{BASE}/armv6m-system-peripherals.svd', 'w') as f:
    f.write('<?xml version="1.0" encoding="utf-8"?>\n')
    f.write(ET.tostring(root_v6, encoding='unicode'))

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
    cpu_name='CM3', revision='r0p1', mpu='false', fpu='false', vtor='true', priobits='3',
    license_src=f'{BASE}/cmsis5/repo/Device/ARM/SVD/ARMCM3.svd')

peripherals7 = ET.SubElement(root_v7, 'peripherals')
peripherals7.append(bg.build_nvic_v7m())
peripherals7.append(bg.build_scb_v7m())
peripherals7.append(bg.build_mpu_v7m())
peripherals7.append(bg.build_systick())
peripherals7.append(bg.build_coredebug(v6m_note=False))

ET.indent(root_v7, space='  ')
with open(f'{BASE}/armv7m-system-peripherals.svd', 'w') as f:
    f.write('<?xml version="1.0" encoding="utf-8"?>\n')
    f.write(ET.tostring(root_v7, encoding='unicode'))

print('wrote both files')

# ---- sanity pass ----
for path in (f'{BASE}/armv6m-system-peripherals.svd', f'{BASE}/armv7m-system-peripherals.svd'):
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
