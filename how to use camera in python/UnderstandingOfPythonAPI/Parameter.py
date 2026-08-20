import xml.etree.ElementTree as ET
from pathlib import Path
import dvp
import csv
import os
import time


class DahengCameraInspector:
    def __init__(self):
        os.environ["GENICAM_GENTL64_PATH"] = r"C:\Program Files (x86)\Do3think\DVP2 x64"
        self.xml_path = r"C:\ProgramData\DO3THINK\DVP2\RGL4K3C.xml"
        self.output_csv = "daheng_params.csv"

    def safe_get_param(self, cam, param_name):
        """安全获取参数值"""
        try:
            value = getattr(cam, param_name)
            # 测试写入（仅对数值型参数）
            if isinstance(value, (int, float)):
                setattr(cam, param_name, value)  # 尝试写回原值
                access = "RW"
            else:
                access = "RO"
            return value, access, True
        except dvp.dvpException as e:
            if e.GetErrorCode() == dvp.DVP_STATUS_PARAMETER_OUT_OF_BOUND:
                return None, "RO", False
            raise
        except:
            return None, "RO", False

    def get_camera_params(self):
        """获取相机参数（带错误处理）"""
        params = []
        try:
            # 1. 刷新相机列表
            cam_list = dvp.Refresh()
            if not cam_list:
                print("⚠️ 未检测到相机！请检查：")
                print("- USB连接和电源")
                print("- 大恒Galaxy Viewer能否识别相机")
                return []

            print(f"找到 {len(cam_list)} 台相机")

            # 2. 连接相机（带超时重试）
            cam = None
            for _ in range(3):  # 最多重试3次
                try:
                    cam = dvp.Camera(0, dvp.OpenMode.OPEN_NORMAL)
                    break
                except dvp.dvpException as e:
                    print(f"相机打开失败（代码 {e.GetErrorCode()}），2秒后重试...")
                    time.sleep(2)

            if not cam:
                print("❌ 相机连接失败！")
                return []

            # 3. 获取参数列表
            all_attrs = [attr for attr in dir(cam)
                         if not attr.startswith('_')
                         and not callable(getattr(cam, attr))]

            # 4. 筛选有效参数
            for attr in all_attrs:
                value, access, is_valid = self.safe_get_param(cam, attr)
                if not is_valid:
                    continue

                param_type = "float" if isinstance(value, float) else \
                    "int" if isinstance(value, int) else \
                        "enum" if hasattr(cam, f"Get{attr}Entries") else \
                            "bool" if isinstance(value, bool) else "string"

                params.append({
                    'name': attr,
                    'type': param_type,
                    'access': access,
                    'value': value,
                    'description': ''
                })

            return params

        except Exception as e:
            print(f"❌ 发生错误: {str(e)}")
            return []
        finally:
            if cam:
                cam.Close()

    def parse_xml_descriptions(self):
        """从XML获取参数描述"""
        try:
            tree = ET.parse(self.xml_path)
            ns = {'genicam': 'http://www.genicam.org/GenApi/Version_1_1'}
            return {
                node.get('Name'): node.find('genicam:Description', ns).text
                for node in tree.findall('.//genicam:*[@Name]', ns)
                if node.find('genicam:Description', ns) is not None
            }
        except Exception as e:
            print(f"⚠️ XML解析警告: {str(e)}")
            return {}

    def run(self):
        """主执行流程"""
        # 获取参数
        params = self.get_camera_params()
        desc_map = self.parse_xml_descriptions()

        # 合并描述
        for p in params:
            p['description'] = desc_map.get(p['name'], "")

        # 打印结果
        if params:
            print("\n✅ 有效参数列表:")
            print("=" * 100)
            print(f"{'参数名':<30} | {'类型':<10} | {'权限':<6} | {'当前值':<15} | 描述")
            print("-" * 100)
            for p in params:
                print(f"{p['name']:<30} | {p['type']:<10} | {p['access']:<6} | "
                      f"{str(p['value']):<15} | {p['description']}")

            # 保存CSV
            with open(self.output_csv, 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(['Parameter', 'Type', 'Access', 'Value', 'Description'])
                writer.writerows([
                    [p['name'], p['type'], p['access'], str(p['value']), p['description']]
                    for p in params
                ])
            print(f"\n结果已保存到: {self.output_csv}")
        else:
            print("❌ 未获取到有效参数！")


if __name__ == "__main__":
    inspector = DahengCameraInspector()
    inspector.run()