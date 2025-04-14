import os
import argparse
import chardet

def detect_and_convert_encoding(file_path, target_encoding):
    # 读取文件二进制内容
    with open(file_path, 'rb') as f:
        raw_data = f.read()

    # 尝试用目标编码解码
    try:
        raw_data.decode(target_encoding)
        print(f"✓ {file_path} 已为 {target_encoding} 编码，跳过")
        return False
    except UnicodeDecodeError:
        pass  # 需要进一步处理

    # 检测文件编码
    detection = chardet.detect(raw_data)
    detected_encoding = detection['encoding']
    confidence = detection['confidence']
    print(f"检测到 {file_path} 可能编码: {detected_encoding} (置信度: {confidence:.2f})")

    # 处理编码别名
    encoding_mapping = {
        'gb2312': 'gbk',
        'iso-8859-1': 'latin_1',
        'ascii': 'utf-8'  # ASCII可视为UTF-8子集
    }
    detected_encoding = encoding_mapping.get(detected_encoding.lower(), detected_encoding)

    # 尝试解码
    decoded = None
    try_encodings = [detected_encoding] if detected_encoding else []
    try_encodings += ['gbk', 'utf-8', 'latin_1', 'cp1252', 'big5', 'shift_jis']

    for enc in try_encodings:
        if not enc:
            continue
        try:
            decoded = raw_data.decode(enc)
            print(f"使用 {enc} 解码成功")
            break
        except (UnicodeDecodeError, LookupError):
            continue

    if not decoded:
        print(f"× 无法解码 {file_path}，支持的编码都尝试失败")
        return False

    # 转换为目标编码
    try:
        encoded_data = decoded.encode(target_encoding)
    except UnicodeEncodeError as e:
        print(f"× 编码失败: {str(e)}")
        return False

    # 写回文件
    try:
        with open(file_path, 'wb') as f:
            f.write(encoded_data)
        print(f"☆ 成功转换 {file_path} 到 {target_encoding}")
        return True
    except IOError as e:
        print(f"× 文件写入失败: {str(e)}")
        return False

def process_directory(directory, target_encoding):
    for root, dirs, files in os.walk(directory):
        for filename in files:
            if filename.endswith(('.cpp', '.h')):
                file_path = os.path.join(root, filename)
                detect_and_convert_encoding(file_path, target_encoding)

def main():
    parser = argparse.ArgumentParser(
        description="自动转换源代码文件编码工具",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument('directory', 
                        help="要处理的目录路径")
    parser.add_argument('-e', '--encoding',
                        default='utf-8',
                        help="目标编码格式（默认：utf-8）\n"
                             "常用格式：utf-8, utf-8-sig, gbk, gb2312")
    args = parser.parse_args()

    if not os.path.isdir(args.directory):
        print(f"错误：{args.directory} 不是有效目录")
        return

    print(f"{'='*40}\n开始处理目录: {args.directory}\n目标编码: {args.encoding}\n{'='*40}")
    process_directory(args.directory, args.encoding)
    print(f"{'='*40}\n处理完成\n{'='*40}")

if __name__ == "__main__":
    main()