import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'screens/profiles_screen.dart';

void main() => runApp(const LarniaApp());

class LarniaApp extends StatelessWidget {
  const LarniaApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Larnia',
      theme: ThemeData(
        useMaterial3: true,
        textTheme: GoogleFonts.poppinsTextTheme(),
        colorScheme: ColorScheme.fromSeed(seedColor: const Color(0xFF125B90)),
      ),
      home: const ProfilesScreen(),
    );
  }
}
