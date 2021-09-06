// Copyright (c) 2019 Vivaldi Technologies AS. All rights reserved

#include "components/request_filter/adblock_filter/adblock_rule_parser.h"

#include <ostream>
#include <string>
#include <vector>

#include "components/request_filter/adblock_filter/adblock_filter_rule.h"
#include "components/request_filter/adblock_filter/parse_result.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace adblock_filter {

TEST(AdBlockRuleParserTest, NothingParsed) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  EXPECT_EQ(AdBlockMetadata(), parse_result.metadata);

  EXPECT_EQ(0u, parse_result.filter_rules.size());
}

TEST(AdBlockRuleParserTest, ParseMetadata) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  EXPECT_EQ(RuleParser::kMetadata,
            rule_parser.Parse("! Homepage: https://vivaldi.com"));
  EXPECT_EQ(RuleParser::kMetadata, rule_parser.Parse("! Title: Test filter"));
  EXPECT_EQ(RuleParser::kMetadata,
            rule_parser.Parse("! Licence: http://www.wtfpl.net/"));
  // Unsupported metadata
  EXPECT_EQ(RuleParser::kComment,
            rule_parser.Parse("! Last modified: 2019-10-08 15:54"));
  EXPECT_EQ(RuleParser::kMetadata, rule_parser.Parse("! Expires: 2 days"));
  EXPECT_EQ(RuleParser::kMetadata, rule_parser.Parse("! Version: 13"));
  EXPECT_EQ(RuleParser::kComment, rule_parser.Parse("! Some other comment"));

  EXPECT_EQ(GURL("https://vivaldi.com"), parse_result.metadata.homepage);
  EXPECT_EQ(GURL("http://www.wtfpl.net/"), parse_result.metadata.license);
  EXPECT_EQ("Test filter", parse_result.metadata.title);
  EXPECT_EQ(13u, parse_result.metadata.version);
  EXPECT_EQ(base::TimeDelta::FromDays(2), parse_result.metadata.expires);

  EXPECT_EQ(0u, parse_result.filter_rules.size());
}

TEST(AdBlockRuleParserTest, SimpleRules) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("badword"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "badword";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("tracker"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "tracker";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("empty-options$"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "empty-options";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("*watchingyou"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "watchingyou";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("**watchingyoutoo"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "watchingyoutoo";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("watchingyouthree*"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "watchingyouthree";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("watchingyoufour**"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "watchingyoufour";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("**watchingyoufive**"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "watchingyoufive";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("*****"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("abc*xyz"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "abc*xyz";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern_type = FilterRule::kWildcarded;
  EXPECT_EQ(RuleParser::kUnsupported, rule_parser.Parse("x"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, RegexRule) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("/(evil|bad)-tracker/"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "(evil|bad)-tracker";
  expected_rules.back().ngram_search_string = "-tracker";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern_type = FilterRule::kRegex;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("/tracker-item-[\\d]+$/"));
  expected_rules.emplace_back();
  expected_rules.back().ngram_search_string = "tracker-item-";
  expected_rules.back().pattern = "tracker-item-[\\d]+$";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern_type = FilterRule::kRegex;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("/tracker-image\\.(png|jpg)$/$image"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "tracker-image\\.(png|jpg)$";
  expected_rules.back().ngram_search_string = "tracker-image.";
  expected_rules.back().resource_types.set(FilterRule::kImage);
  expected_rules.back().party.set();
  expected_rules.back().pattern_type = FilterRule::kRegex;

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("/[xy]+/$script"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "[xy]+";
  expected_rules.back().resource_types.set(FilterRule::kScript);
  expected_rules.back().party.set();
  expected_rules.back().pattern_type = FilterRule::kRegex;

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, BasicAnchors) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("|https://a.bad.domain^"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "https://a.bad.domain^";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorStart);

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("tracker|"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "tracker";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorEnd);

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("|https://a.good.domain/with/*/tracker|"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "https://a.good.domain/with/*/tracker";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern_type = FilterRule::kWildcarded;
  expected_rules.back().anchor_type.set(FilterRule::kAnchorStart);
  expected_rules.back().anchor_type.set(FilterRule::kAnchorEnd);

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("actually_in_the_middle*|"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "actually_in_the_middle";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("|*also_in_the_middle"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = "also_in_the_middle";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, HostAnchors) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("||a.bad.domain.com^"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().host = "a.bad.domain.com";
  expected_rules.back().pattern = "a.bad.domain.com^";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("||another.bad.domain.com"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().host = "another.bad.domain.com";
  expected_rules.back().pattern = "another.bad.domain.com";

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("||vivældi.com"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().host = "xn--vivldi-rua.com";
  expected_rules.back().pattern = "xn--vivldi-rua.com";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("||always.bad.subdomain.*"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().pattern = "always.bad.subdomain.";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("||*not-a-host*/with/path"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern_type = FilterRule::kWildcarded;
  expected_rules.back().pattern = "not-a-host*/with/path";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("||root.of.bad.domain/"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().pattern = "root.of.bad.domain/";

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("||*.bad.subdomains"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = ".bad.subdomains";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("||*.domaine.français"));
  expected_rules.emplace_back();
  expected_rules.back().pattern = ".domaine.xn--franais-xxa";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("||bad_domain.com/æøå"));
  expected_rules.emplace_back();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().pattern = "bad_domain.com/æøå";
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();

  EXPECT_EQ(RuleParser::kUnsupported, rule_parser.Parse("||^nonsense"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, ResourceTypes) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("tracker.jpg$image"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set(FilterRule::kImage);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "tracker.jpg";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("*/tracking-resources/$image,font,media"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set(FilterRule::kImage);
  expected_rules.back().resource_types.set(FilterRule::kMedia);
  expected_rules.back().resource_types.set(FilterRule::kFont);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "/tracking-resources/";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("/images-are-fine$~image"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().resource_types.reset(FilterRule::kImage);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "/images-are-fine";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("only-script-and-css$~script,~stylesheet"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().resource_types.reset(FilterRule::kScript);
  expected_rules.back().resource_types.reset(FilterRule::kStylesheet);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "only-script-and-css";

  EXPECT_EQ(
      RuleParser::kFilterRule,
      rule_parser.Parse("mix-positive-and-negative$~image,~media,script"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().resource_types.reset(FilterRule::kImage);
  expected_rules.back().resource_types.reset(FilterRule::kMedia);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "mix-positive-and-negative";

  EXPECT_EQ(
      RuleParser::kFilterRule,
      rule_parser.Parse(
          "conflicting-positive-and-negative$~image,~media,~font,script,font"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().resource_types.reset(FilterRule::kImage);
  expected_rules.back().resource_types.reset(FilterRule::kMedia);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "conflicting-positive-and-negative";

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, CaseSensitive) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("CaseSensitive$match-case"));
  expected_rules.emplace_back();
  expected_rules.back().is_case_sensitive = true;
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "CaseSensitive";
  expected_rules.back().ngram_search_string = "casesensitive";

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("CaseSensitive"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "casesensitive";

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, Domains) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$domain=some.domain"));
  expected_rules.emplace_back();
  expected_rules.back().included_domains.emplace_back("some.domain");
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(
      RuleParser::kFilterRule,
      rule_parser.Parse(
          "bad-resource$domain=first.domain|second.domain|unicøde.domain"));
  expected_rules.emplace_back();
  expected_rules.back().included_domains.emplace_back("first.domain");
  expected_rules.back().included_domains.emplace_back("second.domain");
  expected_rules.back().included_domains.emplace_back("xn--unicde-eya.domain");
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$domain=~excepted.domain"));
  expected_rules.emplace_back();
  expected_rules.back().excluded_domains.emplace_back("excepted.domain");
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$domain=~first.excepted.domain|~"
                              "second.excepted.domain"));
  expected_rules.emplace_back();
  expected_rules.back().excluded_domains.emplace_back("first.excepted.domain");
  expected_rules.back().excluded_domains.emplace_back("second.excepted.domain");
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$domain=bad.domain|~"
                              "good.bad.domain"));
  expected_rules.emplace_back();
  expected_rules.back().included_domains.emplace_back("bad.domain");
  expected_rules.back().excluded_domains.emplace_back("good.bad.domain");
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("bad-resource$domain=host:port"));
  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("bad-resource$domain=inv/alid"));
  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("bad-resource$domain=wrong]"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, Parties) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$third-party"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set(FilterRule::kThirdParty);
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$third-party,third-party"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set(FilterRule::kThirdParty);
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$~third-party"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set(FilterRule::kFirstParty);
  expected_rules.back().pattern = "bad-resource";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$~third-party,third-party"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, Host) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$host=some.host.name"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";
  expected_rules.back().host = "some.host.name";

  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse(
                "bad-resource$host=some.host.name,host=other.host.name"));
  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("bad-resource$host=[badhost"));
  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("||pattern.host$host=option.host"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, CSP) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$csp=script-src none"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";
  expected_rules.back().is_csp_rule = true;
  expected_rules.back().csp = "script-src none";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("bad-resource$csp=default-src self; img-src *"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-resource";
  expected_rules.back().is_csp_rule = true;
  expected_rules.back().csp = "default-src self; img-src *";

  EXPECT_EQ(
      RuleParser::kFilterRule,
      rule_parser.Parse("@@good-resource$csp=default-src self; img-src *"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "good-resource";
  expected_rules.back().is_csp_rule = true;
  expected_rules.back().is_allow_rule = true;
  expected_rules.back().csp = "default-src self; img-src *";

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("@@good-resource$csp"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "good-resource";
  expected_rules.back().is_csp_rule = true;
  expected_rules.back().is_allow_rule = true;

  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("bad-resource$csp=script-src none; report-uri "
                              "http://report.example.com; img-src none"));
  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("bad-resource$csp=upgrade-insecure-requests"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, Rewrite) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(
      RuleParser::kFilterRule,
      rule_parser.Parse(
          "*bad-script$rewrite=abp-resource:blank-js,domain=some.domain"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-script";
  expected_rules.back().redirect = "blank-js";
  expected_rules.back().included_domains.emplace_back("some.domain");

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("||bad.host/"
                              "bad-image$rewrite=abp-resource:1x1-transparent-"
                              "gif,domain=some.domain"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().pattern = "bad.host/bad-image";
  expected_rules.back().redirect = "1x1-transparent-gif";
  expected_rules.back().included_domains.emplace_back("some.domain");

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse(
                "||tracking.host/"
                "bad-style$rewrite=abp-resource:blank-css,~third-party"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set(FilterRule::kFirstParty);
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().pattern = "tracking.host/bad-style";
  expected_rules.back().redirect = "blank-css";

  EXPECT_EQ(
      RuleParser::kError,
      rule_parser.Parse("*bad-script$rewrite=blank-js,domains=some.domain"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, Redirect) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("*bad-script.js$redirect=noop.js,script"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set(FilterRule::kScript);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "bad-script.js";
  expected_rules.back().redirect = "noop.js";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse(
                "||bad.host/bad-image$redirect=1x1-transparent.gif,image"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set(FilterRule::kImage);
  expected_rules.back().party.set();
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().pattern = "bad.host/bad-image";
  expected_rules.back().redirect = "1x1-transparent.gif";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("||tracking.host/"
                              "bad-file$redirect=empty,~third-party"));
  expected_rules.emplace_back();
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set(FilterRule::kFirstParty);
  expected_rules.back().anchor_type.set(FilterRule::kAnchorHost);
  expected_rules.back().pattern = "tracking.host/bad-file";
  expected_rules.back().redirect = "empty";

  EXPECT_EQ(
      RuleParser::kError,
      rule_parser.Parse(
          "*bad-script$redirect=noop-js,redirect=empty,domains=some.domain"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

TEST(AdBlockRuleParserTest, AllowRuleAndActivation) {
  ParseResult parse_result;
  RuleParser rule_parser(&parse_result);

  std::vector<FilterRule> expected_rules;

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("@@safe-resource"));
  expected_rules.emplace_back();
  expected_rules.back().is_allow_rule = true;
  expected_rules.back().resource_types.set();
  expected_rules.back().party.set();
  expected_rules.back().pattern = "safe-resource";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("|http://this.whole.page$document"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().activation_types.set(FilterRule::kDocument);
  expected_rules.back().anchor_type.set(FilterRule::kAnchorStart);
  expected_rules.back().pattern = "http://this.whole.page";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("@@|http://this.other.page$document"));
  expected_rules.emplace_back();
  expected_rules.back().is_allow_rule = true;
  expected_rules.back().activation_types.set(FilterRule::kDocument);
  expected_rules.back().anchor_type.set(FilterRule::kAnchorStart);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "http://this.other.page";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("@@good-resource$genericblock,generichide"));
  expected_rules.emplace_back();
  expected_rules.back().is_allow_rule = true;
  expected_rules.back().activation_types.set(FilterRule::kGenericBlock);
  expected_rules.back().activation_types.set(FilterRule::kGenericHide);
  expected_rules.back().party.set();
  expected_rules.back().pattern = "good-resource";

  EXPECT_EQ(RuleParser::kError, rule_parser.Parse("not-good$genericblock"));

  EXPECT_EQ(RuleParser::kFilterRule, rule_parser.Parse("distraction$popup"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().activation_types.set(FilterRule::kPopup);
  expected_rules.back().pattern = "distraction";

  EXPECT_EQ(
      RuleParser::kFilterRule,
      rule_parser.Parse("contradictory-activations$popup,~popup,document"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().activation_types.set(FilterRule::kDocument);
  expected_rules.back().pattern = "contradictory-activations";

  EXPECT_EQ(RuleParser::kFilterRule,
            rule_parser.Parse("activations-and-resources$popup,image"));
  expected_rules.emplace_back();
  expected_rules.back().party.set();
  expected_rules.back().activation_types.set(FilterRule::kPopup);
  expected_rules.back().resource_types.set(FilterRule::kImage);
  expected_rules.back().pattern = "activations-and-resources";

  EXPECT_EQ(RuleParser::kError,
            rule_parser.Parse("contradictory-activations$popup,~popup"));

  ASSERT_EQ(expected_rules.size(), parse_result.filter_rules.size());

  auto actual_rules_it = parse_result.filter_rules.begin();
  for (const auto& rule : expected_rules) {
    EXPECT_EQ(rule, *actual_rules_it);
    actual_rules_it++;
  }
}

}  // namespace adblock_filter
