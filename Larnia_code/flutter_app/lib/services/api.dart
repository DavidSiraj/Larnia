import 'dart:convert';
import 'dart:typed_data';
import 'package:http/http.dart' as http;
import '../config.dart';

class LarniaApi {
  final String baseUrl;
  const LarniaApi({this.baseUrl = LarniaConfig.apiBaseUrl});
  Future<List<dynamic>> list(String path) async => jsonDecode((await http.get(Uri.parse('$baseUrl$path'))).body) as List<dynamic>;
  Future<Map<String,dynamic>> get(String path) async => jsonDecode((await http.get(Uri.parse('$baseUrl$path'))).body) as Map<String,dynamic>;
  Future<Map<String,dynamic>> post(String path, Map<String,dynamic> body) async {
    final r = await http.post(Uri.parse('$baseUrl$path'), headers: {'Content-Type':'application/json'}, body: jsonEncode(body));
    if (r.statusCode >= 400) throw Exception(jsonDecode(r.body)['error'] ?? r.body);
    return jsonDecode(r.body) as Map<String,dynamic>;
  }
  Future<Map<String,dynamic>> put(String path, Map<String,dynamic> body) async {
    final r = await http.put(Uri.parse('$baseUrl$path'), headers: {'Content-Type':'application/json'}, body: jsonEncode(body));
    if (r.statusCode >= 400) throw Exception(jsonDecode(r.body)['error'] ?? r.body);
    return jsonDecode(r.body) as Map<String,dynamic>;
  }
  Future<void> delete(String path) async { await http.delete(Uri.parse('$baseUrl$path')); }
  Future<Map<String,dynamic>> uploadMaterial({required String name, required Uint8List bytes, String? teacherId}) async {
    final req = http.MultipartRequest('POST', Uri.parse('$baseUrl/api/materials/upload'));
    if (teacherId != null) req.fields['teacherId'] = teacherId;
    req.files.add(http.MultipartFile.fromBytes('file', bytes, filename: name));
    final res = await req.send();
    final body = await res.stream.bytesToString();
    if (res.statusCode >= 400) throw Exception(jsonDecode(body)['error'] ?? body);
    return jsonDecode(body) as Map<String,dynamic>;
  }
}
