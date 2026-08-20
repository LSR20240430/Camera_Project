import dvp
"""
这行代码是定义了一个函数 test_debug_report_support(camera)，
用来测试相机在不同的报告部分（ReportPart）和报告级别（ReportLevel）下是否支持 DebugReport 功能。
"""
def test_debug_report_support(camera):
    # 输出枚举信息
    print(type(dvp.ReportPart))
    print("📌 ReportPart 枚举值：")
    for name, value in dvp.ReportPart.names.items():
        print(f"{name:15} = {value} (查看值对应的Value字典的整型值: {int(value)})")

    print("\n📌 ReportLevel 枚举值：")
    for name, value in dvp.ReportLevel.names.items():
        print(f"{name:15} = {value} (查看值对应的Value字典的整形值: {int(value)})")

        # 开始测试 DebugReport 支持情况
    parts = dvp.ReportPart.names
    levels = dvp.ReportLevel.names

    print("📊 DebugReport 支持情况（✔️=支持，❌=不支持）：\n")



    """
    dvp.ReportPart.names.items() 返回的是 字典视图对象，它表现得像一个 可迭代的元组，
    每个元素是一个 (键, 值) 元组。
    具体来说，这个视图对象是 dict_items 类型，它可以像列表一样被遍历，但它本质上并不是列表。
    你可以通过迭代它来得到字典中的键值对，每个键值对都是一个元组。
    """
    for part_name, part_enum in parts.items():
        row = f"{part_name:15}: "
        for level_name, level_enum in levels.items():
            try:
                camera.DebugReport(part_enum, level_enum, True, f"Test-{part_name}-{level_name}")
                row += f"[✔️ {level_name}] "
            except Exception as e:
                row += f"[❌ {level_name}*] "
                print(e)
        print(row)
