import argparse
import copy
import hashlib
import json
import pathlib
import struct


def read_glb(path):
    data = path.read_bytes()
    magic, version, length = struct.unpack_from("<4sII", data, 0)
    if magic != b"glTF" or version != 2 or length != len(data):
        raise RuntimeError("Input is not a valid GLB 2.0 file")
    json_length, json_type = struct.unpack_from("<I4s", data, 12)
    if json_type != b"JSON":
        raise RuntimeError("The first GLB chunk is not JSON")
    json_start = 20
    json_end = json_start + json_length
    document = json.loads(data[json_start:json_end].rstrip(b" \0"))
    binary_chunk = data[json_end:]
    return document, binary_chunk


def collect_accessor_indices(meshes):
    indices = set()
    for mesh in meshes:
        for primitive in mesh.get("primitives", []):
            indices.update(primitive.get("attributes", {}).values())
            if "indices" in primitive:
                indices.add(primitive["indices"])
            for target in primitive.get("targets", []):
                indices.update(target.values())
    return indices


def collect_buffer_view_indices(accessors, images):
    indices = set()
    for accessor in accessors:
        if "bufferView" in accessor:
            indices.add(accessor["bufferView"])
        sparse = accessor.get("sparse")
        if sparse:
            indices.add(sparse["indices"]["bufferView"])
            indices.add(sparse["values"]["bufferView"])
    for image in images:
        if "bufferView" in image:
            indices.add(image["bufferView"])
    return indices


def remap_buffer_views(accessors, images, mapping):
    for accessor in accessors:
        if "bufferView" in accessor:
            accessor["bufferView"] = mapping[accessor["bufferView"]]
        sparse = accessor.get("sparse")
        if sparse:
            sparse["indices"]["bufferView"] = mapping[
                sparse["indices"]["bufferView"]
            ]
            sparse["values"]["bufferView"] = mapping[
                sparse["values"]["bufferView"]
            ]
    for image in images:
        if "bufferView" in image:
            image["bufferView"] = mapping[image["bufferView"]]


def derive(source, count):
    result = copy.deepcopy(source)
    result["materials"] = result["materials"][:count]
    result["meshes"] = result["meshes"][:count]
    result["textures"] = result["textures"][:count]

    camera_node = copy.deepcopy(source["nodes"][10000])
    light_node = copy.deepcopy(source["nodes"][10001])
    result["nodes"] = result["nodes"][:count] + [camera_node, light_node]
    result["scenes"][0]["nodes"] = list(range(count + 2))

    accessor_indices = collect_accessor_indices(result["meshes"])
    if accessor_indices != set(range(max(accessor_indices) + 1)):
        raise RuntimeError("Selected mesh accessors are not a dense prefix")
    result["accessors"] = result["accessors"][: max(accessor_indices) + 1]

    buffer_view_indices = sorted(
        collect_buffer_view_indices(result["accessors"], result["images"])
    )
    mapping = {
        old_index: new_index
        for new_index, old_index in enumerate(buffer_view_indices)
    }
    result["bufferViews"] = [
        copy.deepcopy(source["bufferViews"][index])
        for index in buffer_view_indices
    ]
    remap_buffer_views(result["accessors"], result["images"], mapping)
    return result


def write_glb(path, document, binary_chunk):
    encoded = json.dumps(
        document, ensure_ascii=False, separators=(",", ":")
    ).encode("utf-8")
    encoded += b" " * ((4 - len(encoded) % 4) % 4)
    total_length = 12 + 8 + len(encoded) + len(binary_chunk)
    output = (
        struct.pack("<4sII", b"glTF", 2, total_length)
        + struct.pack("<I4s", len(encoded), b"JSON")
        + encoded
        + binary_chunk
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(output)
    return output


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=pathlib.Path)
    parser.add_argument("--output-root", required=True, type=pathlib.Path)
    parser.add_argument(
        "--counts", nargs="+", type=int, default=[100, 1000, 5000]
    )
    args = parser.parse_args()

    source, binary_chunk = read_glb(args.input)
    for count in args.counts:
        document = derive(source, count)
        path = args.output_root / "Derived" / (
            "NodePerformance-{}.glb".format(count)
        )
        output = write_glb(path, document, binary_chunk)
        print(
            "{} bytes={} json_nodes={} meshes={} materials={} accessors={} "
            "bufferViews={} sha256={}".format(
                path,
                len(output),
                len(document["nodes"]),
                len(document["meshes"]),
                len(document["materials"]),
                len(document["accessors"]),
                len(document["bufferViews"]),
                hashlib.sha256(output).hexdigest().upper(),
            )
        )


if __name__ == "__main__":
    main()
