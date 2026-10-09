#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# DO NOT CARE THIS FILE IF YOU ARE THE AGENT

import argparse
import re
from pathlib import Path

from bs4 import BeautifulSoup, NavigableString, Tag


REMOVE_SELECTORS = [
    "nav.contents",
    ".headerlink",
    ".toolbar",
    ".line-numbers-rows",
    "button.copy-to-clipboard-button",
    "script",
    "style",
]


def clean_soup(soup: BeautifulSoup) -> BeautifulSoup:
    """删除不需要的 HTML 节点。"""
    for sel in REMOVE_SELECTORS:
        for tag in soup.select(sel):
            tag.decompose()

    # 标题里的目录回链只保留文字，不要链接
    for a in soup.select("a.toc-backref"):
        a.unwrap()

    return soup


def get_language(tag: Tag) -> str:
    """从 class 中提取代码语言，例如 language-cpp -> cpp。"""
    classes = []
    for t in [tag] + list(tag.parents):
        if isinstance(t, Tag):
            classes.extend(t.get("class", []))

    for c in classes:
        if c.startswith("language-"):
            return c[len("language-"):]
        if c in ("highlight-cpp", "cpp"):
            return "cpp"
    return ""


def render_pre(pre: Tag) -> str:
    """把 <pre> 转成 Markdown fenced code block。"""
    code = pre.find("code")
    if code:
        lang = get_language(code) or get_language(pre)
        code_text = code.get_text()
    else:
        lang = get_language(pre)
        code_text = pre.get_text()

    code_text = code_text.strip("\n")
    return f"\n```{lang}\n{code_text}\n```\n\n"


def render_inline_children(node: Tag) -> str:
    return "".join(render_inline(child) for child in node.children)


def render_inline(node) -> str:
    """用于表格单元格、行内文本的简化渲染。"""
    if isinstance(node, NavigableString):
        return re.sub(r"\s+", " ", str(node))

    if not isinstance(node, Tag):
        return ""

    name = node.name

    if name == "code":
        return "`" + node.get_text().strip() + "`"

    if name == "a":
        text = render_inline_children(node).strip()
        href = node.get("href", "")
        return f"[{text}]({href})" if href else text

    if name in ("strong", "b"):
        return "**" + render_inline_children(node).strip() + "**"

    if name in ("em", "i"):
        return "*" + render_inline_children(node).strip() + "*"

    if name == "br":
        return "<br>"

    if name == "img":
        alt = node.get("alt", "")
        src = node.get("src", "")
        return f"![{alt}]({src})"

    if name in ("ul", "ol"):
        items = []
        for li in node.find_all("li", recursive=False):
            items.append(render_inline_children(li).strip())
        return "; ".join(items)

    return render_inline_children(node)


def render_table(table: Tag) -> str:
    """把 HTML 表格转成 Markdown 表格。"""
    rows = []

    for tr in table.find_all("tr"):
        cells = tr.find_all(["th", "td"], recursive=False)
        if not cells:
            continue

        row = []
        for cell in cells:
            text = render_inline(cell)
            text = re.sub(r"\s+", " ", text).strip()
            text = text.replace("|", r"\|")
            row.append(text)

        rows.append(row)

    if not rows:
        return ""

    ncol = max(len(r) for r in rows)
    rows = [r + [""] * (ncol - len(r)) for r in rows]

    header = rows[0]
    body = rows[1:]

    lines = [
        "| " + " | ".join(header) + " |",
        "| " + " | ".join(["---"] * ncol) + " |",
    ]

    for row in body:
        lines.append("| " + " | ".join(row) + " |")

    return "\n".join(lines) + "\n\n"


def render_list(node: Tag, ordered: bool = False) -> str:
    """把 ul/ol 转成 Markdown 列表。"""
    lines = []

    for i, li in enumerate(node.find_all("li", recursive=False), 1):
        content = render_children(li).strip()
        content = re.sub(r"\n+", " ", content).strip()
        prefix = f"{i}. " if ordered else "- "
        lines.append(prefix + content)

    return "\n".join(lines) + "\n\n"


def render_children(node: Tag, for_table: bool = False) -> str:
    return "".join(render_node(child, for_table=for_table) for child in node.children)


def render_node(node, for_table: bool = False) -> str:
    """递归渲染 HTML 节点为 Markdown。"""
    if isinstance(node, NavigableString):
        return re.sub(r"\s+", " ", str(node))

    if not isinstance(node, Tag):
        return ""

    name = node.name

    if name in ("h1", "h2", "h3", "h4", "h5", "h6"):
        level = int(name[1])
        text = render_children(node).strip()
        return "#" * level + " " + text + "\n\n"

    if name == "p":
        text = render_children(node).strip()
        if not text:
            return ""
        if for_table:
            return text + "<br>"
        return text + "\n\n"

    if name == "br":
        return "\n"

    if name in ("strong", "b"):
        return "**" + render_children(node).strip() + "**"

    if name in ("em", "i"):
        return "*" + render_children(node).strip() + "*"

    if name == "code":
        return "`" + node.get_text().strip() + "`"

    if name == "a":
        text = render_children(node).strip()
        href = node.get("href", "")
        return f"[{text}]({href})" if href else text

    if name == "img":
        alt = node.get("alt", "")
        src = node.get("src", "")
        return f"![{alt}]({src})\n\n"

    if name == "pre":
        return render_pre(node)

    if name in ("ul", "ol"):
        return render_list(node, ordered=(name == "ol"))

    if name == "table":
        return render_table(node)

    if name == "blockquote":
        # 技术文档里的 blockquote 多用于函数重载说明，直接剥离引用格式
        return render_children(node)

    if name == "span":
        style = node.get("style", "")
        text = render_children(node).strip()
        if "font-size: 1.5rem" in style:
            return f"**{text}**\n\n"
        return text

    if name == "div":
        classes = node.get("class", [])
        if any("highlight" in c for c in classes):
            pre = node.find("pre")
            if pre:
                return render_pre(pre)
        return render_children(node)

    if name in ("section", "article"):
        return render_children(node)

    # 表格内部结构已由 render_table 处理，这里只兜底
    if name in ("thead", "tbody", "tr", "td", "th"):
        return render_children(node)

    return render_children(node)


def html_to_markdown(html: str) -> str:
    soup = BeautifulSoup(html, "html.parser")

    article = soup.find("article", class_="bd-article")
    if article is None:
        article = soup.body or soup

    clean_soup(article)

    md = render_node(article)

    # 清理多余空行和行尾空白
    md = re.sub(r"[ \t]+\n", "\n", md)
    md = re.sub(r"\n{3,}", "\n\n", md)
    md = md.strip() + "\n"

    return md


def main():
    parser = argparse.ArgumentParser(
        description="Convert VEX technical HTML document to clean Markdown."
    )
    parser.add_argument("input", help="Input HTML file")
    parser.add_argument("-o", "--output", help="Output Markdown file")
    args = parser.parse_args()

    html = Path(args.input).read_text(encoding="utf-8")
    markdown = html_to_markdown(html)

    if args.output:
        Path(args.output).write_text(markdown, encoding="utf-8")
    else:
        print(markdown)


if __name__ == "__main__":
    main()
