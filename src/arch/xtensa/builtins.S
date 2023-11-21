
// from ESP-IDF
// Apache License 2.0
// float __divsf3(float a, float b) {
.global	__divsf3
.type	__divsf3, @function
__divsf3:
    entry a1, 32
    wfr f0, a2
    wfr f1, a3
    div0.s f3, f1
    nexp01.s f4, f1
    const.s f5, 1
    maddn.s f5, f4, f3
    mov.s f6, f3
    mov.s f7, f1
    nexp01.s f8, f0
    maddn.s f6, f5, f3
    const.s f5, 1
    const.s f2, 0
    neg.s f9, f8
    maddn.s f5, f4, f6
    maddn.s f2, f9, f3
    mkdadj.s f7, f0
    maddn.s f6, f5, f6
    maddn.s f9, f4, f2
    const.s f5, 1
    maddn.s f5, f4, f6
    maddn.s f2, f9, f6
    neg.s f9, f8
    maddn.s f6, f5, f6
    maddn.s f9, f4, f2
    addexpm.s f2, f7
    addexp.s f6, f7
    divn.s f2, f9, f6
    rfr a2, f2
    retw.n