import dvp

"""
CurveLut 的三个属性 enable，style，array 前两个可以被读取和重写，后一个只能被读取，赋值会清0
"""



def create_default_curve_lut() -> dvp.CurveLut:
    """
    创建一个默认的 dvp.CurveLut 对象，包含4个固定点。

    返回:
        配置好的 dvp.CurveLut 对象
    """
    curvelut = dvp.CurveLut()
    curvelut.enable = True
    curvelut.style = dvp.CurveStyle.CURVE_STYLE_LINE

    # 创建4个点
    p1 = dvp.Point2f();
    p1.X, p1.Y = 0.0, 0.0
    p2 = dvp.Point2f();
    p2.X, p2.Y = 0.33, 0.3
    p3 = dvp.Point2f();
    p3.X, p3.Y = 0.66, 0.65
    p4 = dvp.Point2f();
    p4.X, p4.Y = 0.996094, 0.994385

    # 设置 array
    curvelut.array = [[p1, p2, p3, p4]]

    # 4. 验证输出
    print("curve 对象：", curvelut)
    print("array 列表：", curvelut.array)
    return curvelut


    return curvelut