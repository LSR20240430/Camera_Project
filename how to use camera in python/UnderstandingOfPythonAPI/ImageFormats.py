


# 检查该相机原始格式
def list_source_formats_by_enum(camera):
    current_index = camera.SourceFormatSel  # 记录当前索引，之后恢复
    formats = []
    i = 0
    print("可选的原始图像格式：")
    while True:
        try:
            camera.SourceFormatSel = i
            fmt = camera.SourceFormat
            if fmt in formats:
                break
            formats.append(fmt)
            # 标记当前格式
            if i == current_index:
                print(f"{i}: {fmt} (当前格式)")
            else:
                print(f"{i}: {fmt}")
            i += 1
        except:
            break
    camera.SourceFormatSel = current_index  # 恢复原值
    return formats

# 检查该相机目标格式
def list_target_formats_by_enum(camera):
    current_index = camera.TargetFormatSel  # 记录当前索引，之后恢复
    formats = []
    i = 0
    print("可选的目标图像格式：")
    while True:
        try:
            camera.TargetFormatSel = i
            fmt = camera.TargetFormat
            if fmt in formats:
                break
            formats.append(fmt)
            # 标记当前格式
            if i == current_index:
                print(f"{i}: {fmt} (当前格式)")
            else:
                print(f"{i}: {fmt}")
            i += 1
        except:
            break
    camera.TargetFormatSel = current_index  # 恢复原索引
    return formats


# 设置原始格式和目标格式的函数
def set_image_formats(camera, source_index, target_index):
    try:
        # 设置原始图像格式
        camera.SourceFormatSel = source_index
        source_format = camera.SourceFormat
        print(f"成功设置原始图像格式为: {source_format}")

        # 设置目标图像格式
        camera.TargetFormatSel = target_index
        target_format = camera.TargetFormat
        print(f"成功设置目标图像格式为: {target_format}")

    except Exception as e:
        print(f"设置图像格式时出错: {e}")