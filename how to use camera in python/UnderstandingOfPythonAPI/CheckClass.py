import inspect
import re
import pprint
from textwrap import shorten
from collections import defaultdict


def get_instance_info(obj, max_depth=1, _current_depth=0):
    """
    完全保留原始功能，仅增强对Boost.Python对象的处理
    返回对象的详细信息，保持与print(obj)一致的输出格式
    """
    # 深度控制
    if _current_depth >= max_depth:
        return repr(obj)

    try:
        # 特殊处理Boost.Python封装的CurveLut类型
        if hasattr(obj, '__class__') and 'CurveLut' in str(obj.__class__):
            # 手动构建与print完全一致的输出格式
            parts = []
            if hasattr(obj, 'enable'):
                parts.append(f"bool enable = {int(getattr(obj, 'enable'))};")
            if hasattr(obj, 'style'):
                parts.append(f"dvpCurveStyle style = {getattr(obj, 'style')};")
            if hasattr(obj, 'array'):
                array = getattr(obj, 'array')
                array_items = []
                for i in range(len(array)):
                    x = getattr(array[i], 'X', 0)
                    y = getattr(array[i], 'Y', 0)
                    array_items.append(f"{{ float X = {x}; float Y = {y}; }}")
                array_str = "dvpCurveArray { " + " ".join(array_items) + " }"
                parts.append(f"dvpCurveArray array = {array_str};")
            return "{ " + " ".join(parts) + " }"

        # 保留原始处理逻辑
        if hasattr(obj, '__dict__'):
            instance_dict = vars(obj)
            if not instance_dict:
                return repr(obj)
            detailed = {}
            for key, value in instance_dict.items():
                if hasattr(value, '__dict__'):
                    detailed[key] = get_instance_info(value, max_depth, _current_depth + 1)
                else:
                    detailed[key] = value
            return pprint.pformat(detailed, indent=2)

        return repr(obj)
    except Exception:
        return repr(obj)


def check_method_property(obj, analyze_class=False):
    """
    完全保留原始功能，不做任何修改
    深度分析对象或类的所有方法与属性，包含Python和C++签名信息、文档、描述等。
    参数：
        - obj: 目标对象或类
        - analyze_class: 若为 True，则分析类定义（即 type(obj)）；否则分析实例。
    """
    # ---------------------------
    # 定义辅助函数（完全保留原始实现）
    # ---------------------------
    PY_RETURN_REGEX = re.compile(r"->\s*([^:\n]+)")
    CPP_RETURN_REGEX = re.compile(r"^\s*([^\(]+)\s+\w+\s*\(", re.MULTILINE)

    def smart_truncate(text, width=300):
        text = ' '.join(text.split())
        return shorten(text, width, placeholder=' [...]') if text else "无内容"

    def parse_py_return(doc):
        if not doc:
            return "未知"
        match = PY_RETURN_REGEX.search(doc)
        return match.group(1).strip() if match else "void"

    def parse_cpp_return(doc):
        if not doc:
            return "未知"
        match = CPP_RETURN_REGEX.search(doc)
        if match:
            ret_type = match.group(1).strip()
            return ret_type.replace('class ', '').replace('enum ', '')
        return "void"

    def parse_signatures(method, doc):
        py_sig = "无法获取签名"
        py_return = "未知"
        try:
            py_sig = str(inspect.signature(method))
            py_return = parse_py_return(doc)
        except Exception:
            if doc:
                first_line = doc.split('\n')[0]
                if '->' in first_line:
                    py_sig = first_line.split('->')[0].strip()
                    py_return = first_line.split('->')[1].split(':')[0].strip()
        cpp_sig = doc.split('C++ signature :')[-1].strip() if 'C++ signature :' in doc else "无C++签名"
        cpp_return = parse_cpp_return(cpp_sig)
        return {
            'py_sig': py_sig,
            'py_return': py_return,
            'cpp_sig': cpp_sig,
            'cpp_return': cpp_return
        }

    # ---------------------------
    # 分析准备（完全保留原始实现）
    # ---------------------------
    obj_type = type(obj)
    target = obj_type if analyze_class else obj
    target_type = type(target)
    all_attrs = dir(target)

    method_details = []
    method_stats = defaultdict(int)
    processed = set()

    for attr in sorted(all_attrs):
        if attr.startswith('__') or attr in processed:
            continue

        try:
            attr_value = getattr(target, attr)
            if callable(attr_value):
                method = attr_value.__func__ if isinstance(attr_value, (staticmethod, classmethod)) else attr_value
                doc = inspect.getdoc(method) or ""
                sig_info = parse_signatures(method, doc)
                method_type = ('静态方法' if isinstance(attr_value, staticmethod)
                               else '类方法' if isinstance(attr_value, classmethod)
                else '实例方法')
                detail = {
                    'name': attr,
                    'type': method_type,
                    'description': doc.split('\n')[1].strip() if len(doc.split('\n')) > 1 else "无描述",
                    **sig_info,
                    'full_doc': smart_truncate(doc),
                    'source': '继承方法' if attr not in target_type.__dict__ else '类定义方法'
                }
                method_details.append(detail)
                method_stats[method_type] += 1
                method_stats['总计'] += 1
                method_stats[detail['source']] += 1
                processed.add(attr)
        except Exception as e:
            method_details.append({
                'name': attr,
                'type': '访问异常',
                'description': f"属性访问错误: {type(e).__name__}",
                'py_sig': 'N/A',
                'py_return': 'N/A',
                'cpp_sig': 'N/A',
                'cpp_return': 'N/A',
                'full_doc': 'N/A',
                'source': 'N/A'
            })
            method_stats['访问异常'] += 1
            method_stats['总计'] += 1

    # ---------------------------
    # 属性分析（完全保留原始实现）
    # ---------------------------
    attribute_details = []
    attribute_stats = defaultdict(int)

    for attr in sorted(all_attrs):
        if attr.startswith('__') or attr in processed:
            continue

        try:
            attr_value = getattr(target, attr)
            if not callable(attr_value):
                attr_doc = getattr(attr_value, '__doc__', '') or ""
                source = '类定义属性' if attr in target_type.__dict__ else '继承属性'
                attr_type = type(attr_value)
                detailed_type = f"{attr_type.__module__}.{attr_type.__name__}"
                detailed_value = get_instance_info(attr_value, max_depth=1)
                detail = {
                    'name': attr,
                    'type': detailed_type,
                    'description': attr_doc.split('\n')[0].strip() if attr_doc else "无描述",
                    'value': smart_truncate(detailed_value, width=500),
                    'source': source
                }
                attribute_details.append(detail)
                attribute_stats[source] += 1
                attribute_stats['总计'] += 1
        except Exception as e:
            attribute_details.append({
                'name': attr,
                'type': '访问异常',
                'description': f"属性访问错误: {type(e).__name__}",
                'value': 'N/A',
                'source': 'N/A'
            })
            attribute_stats['访问异常'] += 1
            attribute_stats['总计'] += 1

    # ---------------------------
    # 打印报告（完全保留原始实现）
    # ---------------------------
    print(f"\n🔍 深度方法属性分析报告 - {'类定义' if analyze_class else '实例'}：{target_type}")
    print("=" * 80)

    print("\n📊 方法统计:")
    for name, count in [
        ('实例方法', method_stats.get('实例方法', 0)),
        ('静态方法', method_stats.get('静态方法', 0)),
        ('类方法', method_stats.get('类方法', 0)),
        ('类定义方法', method_stats.get('类定义方法', 0)),
        ('继承方法', method_stats.get('继承方法', 0)),
        ('访问异常', method_stats.get('访问异常', 0)),
        ('总计', method_stats.get('总计', 0))
    ]:
        print(f"  {name + ':':<10}{count:>3}个")
    print("=" * 80)

    print("\n📝 方法详情:")
    for detail in method_details:
        output = [
            f"🏷️ 名称: {detail['name']} ({detail['type']}, {detail['source']})",
            f"📌 Python签名: {detail['py_sig']} -> {detail['py_return']}",
            f"🔧 C++签名: {detail['cpp_sig'].split('(')[0]} -> {detail['cpp_return']}",
            f"📋 描述: {detail['description']}",
            f"📚 文档: {detail['full_doc']}",
            "-" * 80
        ]
        print('\n'.join(output))

    print("\n📊 属性统计:")
    for name, count in [
        ('类定义属性', attribute_stats.get('类定义属性', 0)),
        ('继承属性', attribute_stats.get('继承属性', 0)),
        ('访问异常', attribute_stats.get('访问异常', 0)),
        ('总计', attribute_stats.get('总计', 0))
    ]:
        print(f"  {name + ':':<10}{count:>3}个")
    print("=" * 80)

    print("\n📝 属性详情:")
    for detail in attribute_details:
        output = [
            f"🏷️ 名称: {detail['name']} ({detail['type']}, {detail['source']})",
            f"🔍 属性值: {detail['value']}",
            f"📋 描述: {detail['description']}",
            "-" * 80
        ]
        print('\n'.join(output))

# 分隔

# import inspect
# import re
# from textwrap import shorten
# from collections import defaultdict
#
# def check_method_property(obj):
#
#
#     # 增强型正则表达式
#     PY_RETURN_REGEX = re.compile(r"->\s*([^:\n]+)")
#     CPP_RETURN_REGEX = re.compile(r"^\s*([^\(]+)\s+\w+\s*\(", re.MULTILINE)
#
#     # 智能文档处理
#     def smart_truncate(text, width=300):
#         text = ' '.join(text.split())
#         return shorten(text, width, placeholder=' [...]') if text else "无文档"
#
#     # 解析Python返回值
#     def parse_py_return(doc):
#         if not doc:
#             return "未知"
#         match = PY_RETURN_REGEX.search(doc)
#         return match.group(1).strip() if match else "void"
#
#     # 解析C++返回值
#     def parse_cpp_return(doc):
#         if not doc:
#             return "未知"
#         match = CPP_RETURN_REGEX.search(doc)
#         if match:
#             ret_type = match.group(1).strip()
#             # 简化常见类型
#             ret_type = ret_type.replace('class ', '').replace('enum ', '')
#             return ret_type
#         return "void"
#
#     # 增强签名解析
#     def parse_signatures(method, doc):
#         # Python签名
#         py_sig = "无法获取签名"
#         py_return = "未知"
#
#         try:
#             py_sig = str(inspect.signature(method))
#             py_return = parse_py_return(doc)
#         except:
#             if doc:
#                 first_line = doc.split('\n')[0]
#                 if '->' in first_line:
#                     py_sig = first_line.split('->')[0].strip()
#                     py_return = first_line.split('->')[1].split(':')[0].strip()
#
#         # C++签名
#         cpp_sig = doc.split('C++ signature :')[-1].strip() if 'C++ signature :' in doc else "无C++签名"
#         cpp_return = parse_cpp_return(cpp_sig)
#
#         return {
#             'py_sig': py_sig,
#             'py_return': py_return,
#             'cpp_sig': cpp_sig,
#             'cpp_return': cpp_return
#         }
#
#     # 主处理逻辑
#     obj_type = type(obj)
#
#     # 获取所有可访问的方法（包括继承的）
#     all_attrs = dir(obj)
#     method_details = []
#     stats = defaultdict(int)
#     processed = set()
#
#     for attr in sorted(all_attrs):
#         if attr.startswith('__') or attr in processed:
#             continue
#
#         try:
#             attr_value = getattr(obj, attr)
#             if callable(attr_value):
#                 # 获取实际方法对象（解开staticmethod等装饰器）
#                 method = attr_value.__func__ if isinstance(attr_value, (staticmethod, classmethod)) else attr_value
#                 doc = inspect.getdoc(method) or ""
#
#                 # 解析签名信息
#                 sig_info = parse_signatures(method, doc)
#
#                 # 确定方法类型
#                 method_type = '静态方法' if isinstance(attr_value, staticmethod) else \
#                     '类方法' if isinstance(attr_value, classmethod) else \
#                         '实例方法'
#
#                 # 收集方法详情
#                 detail = {
#                     'name': attr,
#                     'type': method_type,
#                     'description': doc.split('\n')[1].strip() if len(doc.split('\n')) > 1 else "无描述",
#                     **sig_info,
#                     'full_doc': smart_truncate(doc),
#                     'source': '继承方法' if attr not in obj_type.__dict__ else '类定义方法'
#                 }
#
#                 method_details.append(detail)
#                 stats[method_type] += 1
#                 stats['总计'] += 1
#                 stats[detail['source']] += 1
#                 processed.add(attr)
#
#         except Exception as e:
#             method_details.append({
#                 'name': attr,
#                 'type': '访问异常',
#                 'description': f"属性访问错误: {type(e).__name__}",
#                 'py_sig': 'N/A',
#                 'py_return': 'N/A',
#                 'cpp_sig': 'N/A',
#                 'cpp_return': 'N/A',
#                 'full_doc': 'N/A',
#                 'source': 'N/A'
#             })
#             stats['访问异常'] += 1
#             stats['总计'] += 1
#
#     # 专业输出格式
#     print(f"\n🔍 深度方法分析报告 - {obj_type}")
#     print("=" * 80)
#
#     # 方法统计
#     print("\n📊 方法统计:")
#     stat_items = [
#         ('实例方法', stats.get('实例方法', 0)),
#         ('静态方法', stats.get('静态方法', 0)),
#         ('类方法', stats.get('类方法', 0)),
#         ('类定义方法', stats.get('类定义方法', 0)),
#         ('继承方法', stats.get('继承方法', 0)),
#         ('访问异常', stats.get('访问异常', 0)),
#         ('总计', stats.get('总计', 0))
#     ]
#
#     for name, count in stat_items:
#         print(f"  {name + ':':<10}{count:>3}个")
#     print("=" * 80)
#
#     # 方法详情
#     print("\n📝 方法详情:")
#     for detail in method_details:
#         output = [
#             f"🏷️ 名称: {detail['name']} ({detail['type']}, {detail['source']})",
#             f"📌 Python签名: {detail['py_sig']} -> {detail['py_return']}",
#             f"🔧 C++签名: {detail['cpp_sig'].split('(')[0]} -> {detail['cpp_return']}",
#             f"📋 描述: {detail['description']}",
#             f"📚 文档: {detail['full_doc']}",
#             "-" * 80
#         ]
#         print('\n'.join(output))


#分隔


# def check_method_property(obj):
#     import inspect
#
#     """
#     检测对象方法的覆盖情况
#     参数:
#         obj: 要检测的Python对象
#     """
#     obj_type = type(obj)
#
#     # 检测__str__是否重写
#     str_overridden = (
#             hasattr(obj_type, '__str__') and
#             getattr(obj_type, '__str__') is not object.__str__
#     )
#
#     # 检测__repr__是否重写
#     repr_overridden = (
#             hasattr(obj_type, '__repr__') and
#             getattr(obj_type, '__repr__') is not object.__repr__
#     )
#
#
#     """
#     要获取类的的构造函数具体信息，因为它是用 Boost.Python 封装的 C++ 类，Python
#     中无法直接用 inspect.signature 拿到签名，所以要结合 SDK 文档信息和试探代码总结来理解。
#     """
#     # 检测__init__是否重写，并获取签名
#     init_func = getattr(obj_type, '__init__', None)
#     init_overridden = init_func is not object.__init__
#
#     try:
#         init_signature = str(inspect.signature(init_func))
#     except Exception as e:
#         init_signature = f"(获取签名失败: {e})"
#
#
#
#     # 获取自定义方法
#     attributes = []
#     custom_methods = []
#     for attr in dir(obj):
#         if not attr.startswith('__'):
#             try:
#                 value = getattr(obj, attr)
#                 if callable(value):  # 判断是否是可调用的属性（即方法）
#                     custom_methods.append(attr)
#                 else:
#                     # 获取属性名字以及它的类型和值
#                     attributes.append((attr, type(value).__name__, value))
#             except Exception as e:
#                 # 处理访问出错的情况
#                 custom_methods.append(f"{attr} (访问出错: {type(e).__name__})")
#
#
#     print("\n===== 对象信息检查 ===== 对象类型为：",obj_type)
#
#     print(f"\n方法重写检查:")
#     print(f"  __str__重写: {str_overridden}")
#     print(f"  __repr__重写: {repr_overridden}")
#     print(f"  __init__ 重写: {init_overridden}")
#     print(f"  __init__签名: {init_signature}")
#
#     # 打印自定义方法
#     print(f"\n自定义方法 ({len(custom_methods)}):")
#     for m in custom_methods:
#         print(f"  - {m}")
#
#     # 打印所有非方法属性
#     print(f"\n--- 打印所有非方法属性 ({len(attributes)}) ---")
#     for attr, tname, val in attributes:
#         print(f"  {attr:<20} (<class '{tname}'>): {val}")
