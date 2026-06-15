import 'package:flutter/material.dart';

const blue = Color(0xFF125B90);
const blue2 = Color(0xFF4C89AF);
const cyan = Color(0xFF82CEDC);
const cyan2 = Color(0xFFA4F0FC);
const errorRed = Color(0xFFCC3E41);

class LarniaMark extends StatelessWidget {
  final double size;
  const LarniaMark({super.key, this.size = 90});
  @override Widget build(BuildContext context) => Image.asset('assets/images/Untitled-1.png', width: size, height: size, fit: BoxFit.contain);
}

class LarniaHeader extends StatelessWidget {
  final String title; final String subtitle; final bool dark;
  const LarniaHeader({super.key, required this.title, this.subtitle = '', this.dark = false});
  @override Widget build(BuildContext context) {
    return Row(crossAxisAlignment: CrossAxisAlignment.start, children: [
      Expanded(child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
        Text(title, style: TextStyle(color: dark ? cyan2 : blue, fontSize: dark ? 40 : 34.22, fontWeight: FontWeight.w500, height: 1.1)),
        if (subtitle.isNotEmpty) const SizedBox(height: 17),
        if (subtitle.isNotEmpty) Text(subtitle, style: const TextStyle(color: blue, fontSize: 12.83, height: 1.5)),
      ])),
      const SizedBox(width: 30),
      Row(children: const [LarniaMark(size: 72), SizedBox(width: 10), Text('Larnia', style: TextStyle(color: blue, fontSize: 28, fontWeight: FontWeight.w600))]),
    ]);
  }
}

class FigmaButton extends StatelessWidget {
  final String text; final Color color; final VoidCallback? onPressed; final double width;
  const FigmaButton({super.key, required this.text, this.color = cyan, this.onPressed, this.width = 215});
  @override Widget build(BuildContext context) => SizedBox(width: width, height: 40, child: ElevatedButton(
    style: ElevatedButton.styleFrom(backgroundColor: color, foregroundColor: Colors.white, shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)), elevation: 0),
    onPressed: onPressed, child: Text(text, style: const TextStyle(fontSize: 16, fontWeight: FontWeight.w500))));
}

class FigmaCard extends StatelessWidget {
  final Widget child; final double width; final double height; final EdgeInsets padding; final VoidCallback? onTap;
  const FigmaCard({super.key, required this.child, required this.width, required this.height, this.padding = const EdgeInsets.all(15), this.onTap});
  @override Widget build(BuildContext context) => InkWell(onTap: onTap, borderRadius: BorderRadius.circular(20), child: Container(
    width: width, height: height, padding: padding,
    decoration: BoxDecoration(color: Colors.white, borderRadius: BorderRadius.circular(20), boxShadow: const [BoxShadow(color: Color(0x26000000), offset: Offset(5,12), blurRadius: 24)]),
    child: child));
}
