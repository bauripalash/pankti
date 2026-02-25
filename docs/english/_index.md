---
title: ""
layout: landing
---
<div class="pb">
    <div class="pb-bg"></div>
    <div class="pbase">
        <p class="pb-main">Pankti</p>
        <p class="pb-sub">Programming Language</p>
        <img src="/script.png" width="800"/>
        <a class="start-btn" href="/bn/docs/">শেখা যাক</a>
    </div>
</div>

# Features of Pankti

{{% columns ratio="1:1" %}}
- ### :desktop_computer: Practical
  Till date multiple attempts had been made to make Bengali Programming Language, some even succeeded. Pankti takes a different approach instead of focusing of educational purpose pankti strives to be a general purpose programming language. From simple 1+2 calculations to complex games, everything is possible with Pankti.

- ### :name_badge: Easy to Learn
  Despite being general purpose the syntax is carefully designed to be easy to learn and use simple everyday words for keywords. Not only Bengali it is possible to use Bengali Phonetic keywords to program if somehow Bengali typing is not available. Anything that can be built with other popular programming languages can be built with Pankti.

- ### :moneybag: Free and Open Source
  Pankti is Free. You don't need to pay anyone anything to use Pankti. The
  source code is also open, you can see how Pankti works internally, make
  changes if you want, I even encourage it. Make programs and share it with
  friends and show your creativity.
{{% /columns %}}


# পঙক্তির বিশেষত্ব

{{% columns ratio="1:1" %}}
- ### :desktop_computer: ব্যবহারিক
  আজ পর্যন্ত বাংলা প্রোগ্রামিং ভাষা তৈরির অনেক প্রচেষ্টা হয়েছে তাদের অনেকেই সফল হয়েছে, কিন্তু পঙক্তি তাদের থেকে আলাদা। পঙক্তি নিছক শিখন উপকরণের ঘেরাটোপের বাইরে বেরিয়ে অন্যান্য প্রচলিত ইংরেজি প্রোগ্রামিং ভাষার মতো সব রকম কাজে ব্যবহারিক হওয়ার চেষ্টা করে। পঙক্তি দিয়ে ১+২ যোগ থেকে শুরু করে অতি জটিল ভিডিও গেম তৈরি করা সম্ভব।
- ### :name_badge: সহজ সরল
  পঙক্তি এমন ভাবে তৈরি করা হয়েছে যাতে সব বয়সের মানুষের কাছে এটি উপযোগী হয়ে উঠে। রোজকার জীবনে ব্যাবহৃত শব্দ এবং সরল বাক্যের দ্বারা পঙক্তিতে প্রোগ্রাম লেখা সম্ভব। শুধু তাই নয়, যদি কোনো কারণে বাংলা ভাষায় লেখা সম্ভব নয় সেখানে ফোনেটিক বাংলা কিংবা ইংরেজি ব্যবহারও সম্ভব। 

- ### :moneybag: সম্পূর্ণ বিনামূল্যে এবং ওপেন সোর্স
  পঙক্তি সম্পূর্ণ বিনামূল্যে ব্যবহার করা যায়। এটি ব্যবহারের জন্য কাউকে কোনো অর্থ দেওয়ার প্রয়োজন নেই। এছাড়া এর সোর্স কোড সম্পূর্ণ উন্মুক্ত। তোমরা নিজেরা দেখতে পারবে, পঙক্তি কেমন করে কাজ করে। তোমার তৈরি প্রোগ্রাম বন্ধুদের মধ্যে ভাগ করে নিজের সৃজনশীলতা ছড়িয়ে দাও। 
{{% /columns %}}

# Examples/উদাহরণ:

{{< tabs >}}

{{% tab "Mathematics - গণিত" %}}

```pankti
// যোগ
১ + ২ // মানঃ ৩

// বিয়োগ
৩ - ১ // মানঃ ২

// গুন
৪ * ৪ //মানঃ ১৬

// ভাগ
৫ / ২ //মানঃ ২.৫
```
{{% /tab %}}

{{% tab "If Else - যদি নাহলে" %}}


```pankti
ধরি বয়স = ৩০
যদি বয়স < ১৮ তাহলে
    দেখাও("নাবালক")
নাহলে
    দেখাও("সাবালক")
শেষ
```

{{% /tab %}}

{{% tab "Loop - লুপ" %}}
```pankti
ধরি ক = ১
যতক্ষণ ক < ৫০ করো
    দেখাও(ক)
    ক = ক + ১
শেষ 
```
{{% /tab %}}

{{% tab "Function - কাজ" %}}

```pankti
কাজ শুভেচ্ছা(নাম)
    দেখাও("নমস্কার " + নাম)
শেষ

শুভেচ্ছা("পলাশ বাউরি")
```

{{% /tab %}}

{{% tab "Arrays - তালিকা" %}}

```pankti
ধরি তালিকা = ["পলাশ বাউরি", "বাংলা", ১০০]

দেখাও(তালিকা[০]) 
```

{{% /tab %}}
{{% tab "HashMaps/Maps - ম্যাপ" %}}

```pankti
ধরি তথা = {
    "নাম" : "পলাশ",
    "পদবী" : "বাউরি",
    "সাল" : ২০২৬, 
}

দেখাও(তথ্য["নাম"]) 
```

{{% /tab %}}


{{< /tabs >}}
