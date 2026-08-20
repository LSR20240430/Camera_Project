import xml.etree.ElementTree as ET

# 读取 GenICam XML 文件（路径请替换成你自己的）
xml_path = r"C:\ProgramData\Camera\DVP2\SGC130M-H_20250402101211\MGV130.xml"
tree = ET.parse(xml_path)
root = tree.getroot()

# 获取命名空间前缀（如 {http://www.genicam.org/GenApi/Version_1_1}）
namespace_prefix = root.tag.split("}")[0] + "}"


# 递归打印 XML 节点信息（带中文注释和结构）
def print_node(node, level=0):
    indent = "  " * level  # 缩进
    tag = node.tag.split("}")[-1]  # 去掉命名空间
    attrs = node.attrib
    text = (node.text or "").strip()

    # 打印标签名
    print(f"{indent}标签: {tag}")

    # 打印属性（如果有）
    if attrs:
        print(f"{indent}属性: {attrs}")
    else:
        print(f"{indent}无属性")

    # 打印文本内容（如果有）
    if text:
        print(f"{indent}文本内容: {text}")
    else:
        print(f"{indent}无文本")

    # 如果有子标签，继续递归
    for child in node:
        print_node(child, level + 1)


# 打印根节点及其子节点
print(f"根节点: {root.tag}")
print_node(root)
