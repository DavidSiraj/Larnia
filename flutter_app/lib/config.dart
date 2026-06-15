class LarniaConfig {
  static const String apiBaseUrl = String.fromEnvironment(
    'LARNIA_API_BASE_URL',
    defaultValue: 'http://192.168.137.1:3000',
  );
}
