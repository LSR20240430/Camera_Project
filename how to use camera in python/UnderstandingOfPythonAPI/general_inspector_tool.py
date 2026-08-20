# general_inspector_tool.py
"""
通用对象检测工具 - 增强版
========================

功能特性：
1. 深度分析对象/类的属性与方法
2. 自动识别C++ Boost.Python封装对象
3. 智能递归展开嵌套结构
4. 自动调用getter方法探测状态
5. 生成结构化分析报告

使用示例：
>>> from general_inspector_tool import analyze
>>> analyze(your_object)
>>> analyze(your_object, analyze_class=True, max_depth=2)
"""

import inspect
import pprint
import logging
import re
from textwrap import shorten
from collections import defaultdict

# ================================
# 日志配置（非侵入式）
# ================================
_logger = logging.getLogger(__name__)
_logger.addHandler(logging.NullHandler())


# ================================
# 模块A：核心辅助函数
# ================================
def try_call_getters(obj):
    """
    自动调用对象的所有getter方法
    返回格式：{方法名: 返回值/错误信息}
    """
    result = {}
    for name, method in inspect.getmembers(obj, predicate=inspect.ismethod):
        if name.lower().startswith("get"):
            try:
                if method.__code__.co_argcount == 1:
                    result[name] = method()
            except Exception as e:
                result[name] = f"<调用失败: {e}>"
    return result


def deep_describe(obj, depth=0, max_depth=2):
    """
    递归展开对象结构
    返回易读的嵌套字典结构
    """
    if isinstance(obj, (bool, int, float, complex, str, bytes)) or obj is None:
        return repr(obj)

    if depth >= max_depth:
        return repr(obj)

    desc = {}
    if hasattr(obj, '__dict__'):
        for key, val in vars(obj).items():
            try:
                desc[key] = deep_describe(val, depth + 1, max_depth)
            except Exception as e:
                desc[key] = f"<无法展开: {e}>"
    elif hasattr(obj, '__slots__'):
        slots = getattr(obj, '__slots__')
        if isinstance(slots, str):
            slots = [slots]
        for slot in slots:
            try:
                val = getattr(obj, slot)
                desc[slot] = deep_describe(val, depth + 1, max_depth)
            except Exception as e:
                desc[slot] = f"<无法展开: {e}>"
    else:
        try:
            s = str(obj)
            if not (s.startswith("<") and s.endswith(">")):
                return s
        except Exception:
            pass
        return repr(obj)
    return desc


# ================================
# 模块B：对象结构解析
# ================================
def get_instance_info(obj, max_depth=1, _current_depth=0):
    """
    获取对象的详细结构信息
    返回格式化字符串
    """
    if isinstance(obj, (bool, int, float, complex, str, bytes)):
        return repr(obj)

    if _current_depth >= max_depth:
        return repr(obj)

    try:
        if hasattr(obj, '__dict__'):
            instance_dict = vars(obj)
            if instance_dict:
                detailed = {}
                for key, value in instance_dict.items():
                    detailed[key] = get_instance_info(value, max_depth, _current_depth + 1)
                return pprint.pformat(detailed, indent=2)

        if hasattr(obj, '__slots__'):
            slots = getattr(obj, '__slots__')
            detailed = {}
            if isinstance(slots, str):
                slots = [slots]
            for slot in slots:
                try:
                    value = getattr(obj, slot)
                    detailed[slot] = get_instance_info(value, max_depth, _current_depth + 1)
                except Exception as e:
                    detailed[slot] = f"<获取失败: {type(e).__name__}>"
            return pprint.pformat(detailed, indent=2)

        if hasattr(obj, '__class__') and 'Boost.Python' in str(obj.__class__):
            members = [attr for attr in dir(obj) if not attr.startswith('__') and not callable(getattr(obj, attr))]
            details = {}
            for attr in members:
                try:
                    val = getattr(obj, attr)
                    details[attr] = get_instance_info(val, max_depth, _current_depth + 1)
                except Exception as e:
                    details[attr] = f'<获取失败: {type(e).__name__}>'
            indent = '  ' * _current_depth
            content = []
            for k, v in details.items():
                content.append(f"{indent}  {k} = {v};")
            return f"{obj.__class__.__name__} {{\n" + '\n'.join(content) + f"\n{indent}}}"

        return repr(obj)
    except Exception:
        return f"<{obj.__class__.__name__} at 0x{id(obj):x}>"


# ================================
# 模块C：方法属性分析
# ================================
def check_method_property(obj, analyze_class=False):
    """
    核心分析方法与属性
    返回结构化分析结果
    """
    PY_RETURN_REGEX = re.compile(r"->\s*([^:\n]+)")
    CPP_RETURN_REGEX = re.compile(r"^\s*([^\(]+)\s+\w+\s*\(", re.MULTILINE)

    def smart_truncate(text, width=300):
        text = ' '.join(text.split())
        return shorten(text, width, placeholder=' [...]') if text else "无内容"

    def parse_signatures(method, doc):
        try:
            py_sig = str(inspect.signature(method))
            py_return = PY_RETURN_REGEX.search(doc).group(1).strip() if doc else "void"
        except Exception:
            py_sig = "无法获取签名"
            py_return = "未知"

        cpp_sig = doc.split('C++ signature :')[-1].strip() if 'C++ signature :' in doc else "无C++签名"
        cpp_return = CPP_RETURN_REGEX.search(cpp_sig).group(1).strip() if CPP_RETURN_REGEX.search(cpp_sig) else "void"

        return {
            'py_sig': py_sig,
            'py_return': py_return,
            'cpp_sig': cpp_sig,
            'cpp_return': cpp_return
        }

    target = type(obj) if analyze_class else obj
    all_attrs = dir(target)

    method_details = []
    method_stats = defaultdict(int)
    attribute_details = []
    attribute_stats = defaultdict(int)

    # 分析方法
    for attr in all_attrs:
        if attr.startswith('__'):
            continue
        try:
            attr_value = getattr(target, attr)
            if callable(attr_value):
                doc = inspect.getdoc(attr_value) or ""
                sig_info = parse_signatures(attr_value, doc)
                method_type = ('静态方法' if isinstance(attr_value, staticmethod)
                               else '类方法' if isinstance(attr_value, classmethod)
                else '实例方法')
                method_details.append({
                    'name': attr,
                    'type': method_type,
                    **sig_info,
                    'description': doc.splitlines()[1].strip() if len(doc.splitlines()) > 1 else "无描述",
                    'source': '继承方法' if attr not in target.__dict__ else '类定义方法'
                })
                method_stats[method_type] += 1
        except Exception as e:
            method_details.append({
                'name': attr,
                'type': '访问异常',
                'description': f"属性访问错误: {type(e).__name__}",
                **dict.fromkeys(['py_sig', 'py_return', 'cpp_sig', 'cpp_return'], 'N/A')
            })
            method_stats['访问异常'] += 1

    # 分析属性
    for attr in all_attrs:
        if attr.startswith('__') or callable(getattr(target, attr)):
            continue
        try:
            attr_value = getattr(target, attr)
            attr_type = type(attr_value)
            attribute_details.append({
                'name': attr,
                'type': f"{attr_type.__module__}.{attr_type.__name__}",
                'value': smart_truncate(str(attr_value)),
                'source': '类定义属性' if attr in target.__dict__ else '继承属性'
            })
            attribute_stats[attribute_details[-1]['source']] += 1
        except Exception as e:
            attribute_details.append({
                'name': attr,
                'type': '访问异常',
                'value': f"属性访问错误: {type(e).__name__}",
                'source': 'N/A'
            })
            attribute_stats['访问异常'] += 1

    return {
        'target_type': type(target),
        'method_details': method_details,
        'method_stats': dict(method_stats),
        'attribute_details': attribute_details,
        'attribute_stats': dict(attribute_stats)
    }


# ================================
# 模块D：报告生成
# ================================
def print_report(analysis, analyze_class=False):
    """
    生成易读的分析报告
    """
    print(f"\n🔍 深度分析报告 - {'类' if analyze_class else '实例'}: {analysis['target_type']}")
    print("=" * 80)

    # 方法统计
    print("\n📊 方法统计:")
    stats = analysis['method_stats']
    for k in ['实例方法', '静态方法', '类方法', '类定义方法', '继承方法', '访问异常', '总计']:
        if k in stats:
            print(f"  {k + ':':<12}{stats[k]:>3}")

    # 方法详情
    print("\n📝 方法详情:")
    for m in analysis['method_details']:
        print(f"  {m['name']} ({m['type']})")
        print(f"    Python签名: {m['py_sig']} -> {m['py_return']}")
        print(f"    C++签名:    {m['cpp_sig']} -> {m['cpp_return']}")
        print(f"    来源:       {m['source']}")
        print("-" * 60)

    # 属性统计
    print("\n📊 属性统计:")
    stats = analysis['attribute_stats']
    for k in ['类定义属性', '继承属性', '访问异常', '总计']:
        if k in stats:
            print(f"  {k + ':':<12}{stats[k]:>3}")

    # 属性详情
    print("\n📝 属性详情:")
    for a in analysis['attribute_details']:
        print(f"  {a['name']} ({a['type']})")
        print(f"    值:    {a['value']}")
        print(f"    来源:  {a['source']}")
        print("-" * 60)


# ================================
# 模块E：统一接口
# ================================
def analyze(obj, analyze_class=False, max_depth=1, deep_inspect=False):
    """
    一站式分析入口

    参数:
        obj: 要分析的目标对象
        analyze_class: 是否分析类定义 (默认False)
        max_depth: 递归展开深度 (默认1)
        deep_inspect: 是否深度检查 (默认False)
    """
    analysis = check_method_property(obj, analyze_class)
    print_report(analysis, analyze_class)

    if deep_inspect:
        print("\n🔍 深度结构解析:")
        pprint.pprint(deep_describe(obj, max_depth=max_depth), width=120)

    if not analyze_class:
        if getters := try_call_getters(obj):
            print("\n🔧 Getter调用结果:")
            pprint.pprint(getters, width=120)


if __name__ == '__main__':
    print("请作为库使用：from general_inspector_tool import analyze")