import dvp
import inspect
import traceback
import types
import re
from colorama import Fore, init

# 初始化colorama
init(autoreset=True)


def get_enum_values():
    """获取 dvp 模块中定义的所有枚举常量。"""
    enum_dict = {}
    for name in dir(dvp):
        if name.isupper():
            try:
                value = getattr(dvp, name)
                enum_dict[name] = value
            except Exception:
                continue
    return enum_dict


def get_supported_values_from_enum(attribute_name, enum_dict):
    """尝试根据属性名称猜测可能的枚举项。"""
    matches = []
    attr_upper = attribute_name.upper()
    for name, value in enum_dict.items():
        if attr_upper in name:
            matches.append((name, value))
    return matches


def is_method_overridden(method_name, obj):
    """检查方法是否被重写。"""
    base_classes = inspect.getmro(type(obj))
    for base in base_classes[1:]:
        if hasattr(base, method_name):
            base_method = getattr(base, method_name, None)
            if base_method and isinstance(base_method, (types.MethodType, types.BuiltinMethodType, types.FunctionType)):
                return True
    return False


def extract_cpp_signature(doc_text):
    """从方法文档中提取 C++ 签名。"""
    if not doc_text:
        return None

    # 尝试匹配 C++ 签名模式
    cpp_pattern = r"C\+\+\s*signature\s*:\s*(.*)"
    match = re.search(cpp_pattern, doc_text, re.IGNORECASE)
    if match:
        return match.group(1).strip()

    # 尝试匹配 enum 开头的签名
    enum_pattern = r"enum\s+\w+\s+\w+\(.*\)"
    match = re.search(enum_pattern, doc_text)
    if match:
        return match.group(0)

    return None


def extract_python_signature_from_doc(method_name, doc_text):
    """从文档字符串中提取 Python 签名。"""
    if not doc_text:
        return None

    # 第一行通常包含签名信息
    first_line = doc_text.strip().split('\n')[0]

    # 匹配类似 "Stop( (Camera)arg1) -> Status :" 的格式
    if method_name in first_line and '(' in first_line and ')' in first_line:
        # 去除可能的多余描述
        signature = first_line.split(':')[0].strip()
        # 确保包含方法名
        if signature.startswith(method_name):
            return signature

    return None


def format_python_signature(signature):
    """格式化Python签名使其更美观"""
    if not signature:
        return None

    # 基本清理
    signature = signature.strip()

    # 修复方法名后的空格问题
    signature = re.sub(r'(\w+)\s*\(', r'\1(', signature)

    # 处理可选参数
    signature = re.sub(r'\[\s*,\s*', ', ', signature)
    signature = re.sub(r'\[\s*', '(', signature)
    signature = re.sub(r'\]\s*', ')', signature)

    # 确保括号匹配
    if signature.count('(') > signature.count(')'):
        signature += ')'

    # 统一参数格式
    signature = re.sub(r'\(\s*', '(', signature)
    signature = re.sub(r'\s*\)', ')', signature)
    signature = re.sub(r'\s*,\s*', ', ', signature)

    # 美化返回箭头
    signature = re.sub(r'\s*->\s*', ' -> ', signature)

    return signature


def format_cpp_signature(signature):
    """格式化C++签名使其更美观"""
    if not signature:
        return None

    # 简化类型声明
    signature = signature.replace('class ', '')
    signature = signature.replace('struct ', '')
    signature = signature.replace('enum ', '')
    signature = signature.replace('__ptr64', '')
    signature = signature.replace('{lvalue}', '')

    # 处理可选参数
    signature = re.sub(r'\[\s*,\s*', ', ', signature)
    signature = re.sub(r'\[\s*', '(', signature)
    signature = re.sub(r'\]\s*', ')', signature)

    # 统一格式
    signature = re.sub(r'\(\s*', '(', signature)
    signature = re.sub(r'\s*\)', ')', signature)
    signature = re.sub(r'\s*,\s*', ', ', signature)

    # 移除多余空格
    signature = re.sub(r'\s+', ' ', signature).strip()

    return signature


def analyze_attribute(attr_name, attr, enum_dict, obj):
    """分析单个属性并打印信息。"""
    print(f"\n{Fore.GREEN}🏷️ 名称: {attr_name} ({type(attr).__module__}.{type(attr).__name__}){Fore.RESET}")

    try:
        print(f"🔍 属性值: {attr}")
    except Exception as e:
        print(f"  ❌ 获取属性值失败: {e}")

    # 支持的枚举值
    supported = get_supported_values_from_enum(attr_name, enum_dict)
    if supported:
        print(f"{Fore.CYAN}📚 支持的枚举值:{Fore.RESET}")
        for name, val in supported:
            print(f"   {name} = {val}")

    # 属性描述信息
    doc = getattr(type(obj), attr_name, None)
    doc_text = getattr(doc, '__doc__', '') or getattr(attr, '__doc__', '无描述')
    if doc_text and doc_text != '无描述':
        print(f"{Fore.YELLOW}📋 描述:{Fore.RESET} {doc_text.strip().splitlines()[0]}")


def analyze_method(method_name, method, obj):
    """分析单个方法并打印信息。"""
    print(f"\n{Fore.BLUE}🏷️ 名称: {method_name}{Fore.RESET}")

    # 获取文档字符串
    doc_text = getattr(method, '__doc__', '')

    # 提取并格式化Python签名
    py_sig = extract_python_signature_from_doc(method_name, doc_text)
    py_sig = format_python_signature(py_sig)
    if py_sig:
        print(f"{Fore.CYAN}📌 Python签名:{Fore.RESET} {py_sig}")
    else:
        print(f"{Fore.YELLOW}📌 Python签名: 无法获取{Fore.RESET}")

    # 提取并格式化C++签名
    cpp_sig = extract_cpp_signature(doc_text)
    cpp_sig = format_cpp_signature(cpp_sig)
    if cpp_sig:
        print(f"{Fore.MAGENTA}🔧 C++签名:{Fore.RESET} {cpp_sig}")
    else:
        print(f"{Fore.YELLOW}🔧 C++签名: 无法提取{Fore.RESET}")

    # 检查方法是否被重写
    if is_method_overridden(method_name, obj):
        print(f"{Fore.YELLOW}🔧 此方法已被重写！{Fore.RESET}")

    # 打印方法文档
    if doc_text:
        lines = [line.strip() for line in doc_text.split('\n') if line.strip()]
        if lines:
            # 查找真正的描述（非签名行）
            description = None
            for line in lines:
                if not line.startswith(method_name) and 'signature' not in line.lower():
                    description = line
                    break

            if description:
                print(f"{Fore.GREEN}📋 描述:{Fore.RESET} {description}")

            # 打印额外的文档信息
            extra_docs = [line for line in lines
                          if line != description
                          and not line.startswith(method_name)
                          and 'signature' not in line.lower()]
            for doc in extra_docs:
                print(f"   {Fore.WHITE}📄 {doc}{Fore.RESET}")


def analyze_instance(obj, obj_name="对象"):
    """分析对象的属性和方法。"""
    print(f"\n{'-' * 50}")
    print(f"{Fore.YELLOW}🔍 分析 {obj_name} 的属性和方法:{Fore.RESET}")
    enum_dict = get_enum_values()
    attributes = []
    methods = []

    for attr_name in dir(obj):
        if attr_name.startswith("_"):
            continue
        try:
            attr = getattr(obj, attr_name)
            if isinstance(attr, (types.MethodType, types.BuiltinMethodType, types.FunctionType)):
                methods.append((attr_name, attr))
            else:
                attributes.append((attr_name, attr))
        except Exception as e:
            print(f"\n{Fore.RED}🏷️ 名称: {attr_name}")
            print(f"  ❌ 获取失败: {e}{Fore.RESET}")

    # 分析属性
    attribute_count = 0
    for attr_name, attr in attributes:
        attribute_count += 1
        analyze_attribute(attr_name, attr, enum_dict, obj)

    # 分析方法
    method_count = 0
    for method_name, method in methods:
        method_count += 1
        analyze_method(method_name, method, obj)

    # 打印统计信息
    print(f"\n{'-' * 50}")
    print(
        f"{Fore.CYAN}📊 总计:{Fore.RESET} {Fore.GREEN}属性 {attribute_count} 个{Fore.RESET}, {Fore.BLUE}方法 {method_count} 个{Fore.RESET}")
    print(f"{'-' * 50}")


def analyze_all_classes(module):
    """分析模块中的所有类"""
    print(f"\n{Fore.YELLOW}=== 开始分析模块中的所有类 ==={Fore.RESET}")

    for name, obj in inspect.getmembers(module):
        if inspect.isclass(obj) and obj.__module__ == module.__name__:
            try:
                print(f"\n{Fore.CYAN}=== 分析类 {name} ===")
                print(f"模块: {obj.__module__}")
                print(f"文档: {obj.__doc__ or '无文档'}")

                # 尝试创建实例并分析
                try:
                    instance = obj()
                    analyze_instance(instance, f"{name} 实例")
                except Exception as e:
                    print(f"{Fore.RED}无法创建实例: {e}{Fore.RESET}")
                    # 如果无法创建实例，直接分析类对象
                    analyze_instance(obj, f"{name} 类")

            except Exception as e:
                print(f"\n{Fore.RED}分析类 {name} 时出错: {e}{Fore.RESET}")
                traceback.print_exc()

    print(f"\n{Fore.GREEN}✅ 类分析完成{Fore.RESET}")


def main():
    try:
        print(f"{Fore.YELLOW}=== 开始分析 dvp 模块 ==={Fore.RESET}")

        # 分析相机列表
        camera_list = dvp.Refresh()
        if not camera_list:
            print(f"{Fore.RED}没有找到相机{Fore.RESET}")
        else:
            # 分析相机信息
            cam_info = camera_list[0]
            analyze_instance(cam_info, "CameraInfo 实例")

            # 分析相机对象
            camera = dvp.Camera(0, dvp.OpenMode.OPEN_NORMAL)
            analyze_instance(camera, "Camera 实例")

        # 分析dvp模块中的所有类
        analyze_all_classes(dvp)

        print(f"\n{Fore.GREEN}✅ 分析完成{Fore.RESET}")

    except Exception as e:
        print(f"\n{Fore.RED}❌ 程序异常:{Fore.RESET}")
        traceback.print_exc()


if __name__ == '__main__':
    main()